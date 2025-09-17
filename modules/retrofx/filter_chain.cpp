//
// Created by Stuart Carnie on 7/8/2024.
//

#include "filter_chain.h"

#include "core/io/file_access_memory.h"
#include "core/io/image_loader.h"
#include "servers/rendering/renderer_rd/storage_rd/texture_storage.h"
#include "servers/rendering/renderer_rd/uniform_set_cache_rd.h"

static_assert(sizeof(real_t) == 4, "real_t should be single-precision float");

FilterChain::Vertex FilterChain::vertex[4] = {
	{ .position = Vector4(0, 1, 0, 1), .tex_coord = Vector2(0, 1) },
	{ .position = Vector4(1, 1, 0, 1), .tex_coord = Vector2(1, 1) },
	{ .position = Vector4(0, 0, 0, 1), .tex_coord = Vector2(0, 0) },
	{ .position = Vector4(1, 0, 0, 1), .tex_coord = Vector2(1, 0) },
};

Size2i FilterChain::Pass::get_output_size(Size2i p_viewport, Size2i p_source) const {
	double width = p_source.width;
	if (scale_x.has_value()) {
		switch (scale_x->type) {
			case compiled::ScaleType::Source:
				width *= scale_x->scale;
				break;
			case compiled::ScaleType::Absolute:
				width = scale_x->size;
				break;
			case compiled::ScaleType::Viewport:
				width = p_viewport.width * scale_x->scale;
				break;
		}
	}

	double height = p_source.height;
	if (scale_y.has_value()) {
		switch (scale_y->type) {
			case compiled::ScaleType::Source:
				height *= scale_y->scale;
				break;
			case compiled::ScaleType::Absolute:
				height = scale_y->size;
				break;
			case compiled::ScaleType::Viewport:
				height = p_viewport.height * scale_y->scale;
				break;
		}
	}

	return Size2i(Math::round(width), Math::round(height));
}

Rect2 FilterChain::fit_aspect_rect_into_rect(Size2 p_aspect, Size2 p_size) {
	double want_aspect = (double)p_aspect.width / (double)p_aspect.height;
	double view_aspect = (double)p_size.width / (double)p_size.height;

	double min_factor;
	Size2 out_rect_size;
	if (view_aspect >= want_aspect) {
		// Raw image is too wide (normal case), squish inwards
		min_factor = want_aspect / view_aspect;
		out_rect_size = Size2(p_size.width * min_factor, p_size.height);
	} else {
		// Raw image is too tall, squish upwards
		min_factor = view_aspect / want_aspect;
		out_rect_size = Size2(p_size.width, p_size.height * min_factor);
	}

	// round origin outwards to nearest pixel
	Point2 origin = Point2(
			Math::floor((p_size.width - out_rect_size.width) / 2.0),
			Math::floor((p_size.height - out_rect_size.height) / 2.0));
	// round size outwards to nearest pixel
	Size2 size = Size2(Math::ceil(out_rect_size.width), Math::ceil(out_rect_size.height));
	return Rect2(origin, size);
}

void FilterChain::resize() {
	Rect2 bounds = fit_aspect_rect_into_rect(aspect_size, drawable_size);
	if (bounds == output_bounds) {
		return;
	}

	output_bounds = bounds;
	output_frame.viewport = output_bounds;
	output_frame.size = TextureSize(output_bounds.size);

	if (has_shader) {
		render_targets_need_resize = true;
	}
}

void FilterChain::set_source_rect(Rect2 p_rect, Size2 p_aspect) {
	if (source_rect == p_rect && aspect_size == p_aspect) {
		return;
	}

	source_rect = p_rect;
	aspect_size = p_aspect;
	resize();
}

void FilterChain::set_drawable_size(Size2 p_size) {
	if (drawable_size == p_size) {
		return;
	}

	drawable_size = p_size;
	resize();
}

void FilterChain::set_parameter_value(uint32_t p_index, double p_value) {
	ERR_FAIL_COND_MSG(p_index >= parameters_count, "Parameter index out of range");

	parameters[p_index] = p_value;
}

void FilterChain::set_parameter_value(const String &p_name, double p_value) {
	uint32_t *index = parameters_map.getptr(p_name);
	ERR_FAIL_NULL_MSG(index, "Parameter not found.");

	parameters[*index] = p_value;
}

bool FilterChain::get_parameter_value(uint32_t p_index, double &r_value) const {
	if (p_index >= parameters_count) {
		return false;
	}

	r_value = parameters[p_index];
	return true;
}

bool FilterChain::get_parameter_value(const String &p_name, double &r_value) const {
	const uint32_t *index = parameters_map.getptr(p_name);
	if (!index) {
		return false;
	}

	r_value = parameters[*index];
	return true;
}

void FilterChain::resize_render_targets() {
	DEV_ASSERT(render_targets_need_resize);

	RD *rd = RD::get_singleton();

	// current source size
	Size2i source_size = source_rect.size;
	Size2i viewport_size = output_frame.viewport.size;

	for (uint32_t i = 0; i < passes_count; i++) {
		Pass &pass = passes[i];
		pass.uniform_set = RID(); // clear it on resize, as it will need to be updated

		Size2i pass_size;
		if (!pass.is_scaled()) {
			pass_size = i == last_pass_index ? viewport_size : source_size;
		} else {
			pass_size = pass.get_output_size(viewport_size, source_size);
		}

		// capture the source size for the next pass
		source_size = pass_size;

		print_line(vformat("pass %d, render target size %d x %d", i, pass_size.width, pass_size.height));

		RD::DataFormat fmt = pass.format;
		// TODO(sgc): capture render target format so this is correct
		if (i == last_pass_index && pass_size == viewport_size && pass.format == RD::DATA_FORMAT_R8G8B8A8_UNORM) {
			// last pass can render directly to the output render target
			pass.render_target.texture.size = TextureSize(pass_size);
		} else {
			pass.viewport = Rect2i(Point2i(), pass_size);

			if (pass.render_target.is_valid() && pass.render_target.width() == pass_size.width && pass.render_target.height() == pass_size.height) {
				// render target is already the correct size
				continue;
			}

			RD::TextureFormat tf;
			tf.format = fmt;
			tf.width = pass_size.width;
			tf.height = pass_size.height;
			tf.usage_bits = RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_COPY_TO_BIT;
			tf.texture_type = RD::TEXTURE_TYPE_2D;

			pass.render_target.free(rd);
			pass.feedback_target.free(rd);

			String label = vformat("Pass %02d Output", i);

			pass.render_target.init(rd, tf, pass.fb_format);
			rd->set_resource_name(pass.render_target.texture.rid, label);

			if (pass.has_feedback) {
				pass.feedback_target.free(rd);
				pass.feedback_target.init(rd, tf, pass.fb_format);
				rd->set_resource_name(pass.render_target.texture.rid, label);
			}
		}
	}

	render_targets_need_resize = false;
}

void FilterChain::init_next_history_texture() {
	CRASH_COND_MSG(history_count == 0, "Shader does not require history textures");

	// either no history, or we moved a texture of a different size in the front slot
	if (history_textures[0].size.width() != source_rect.size.width || history_textures[0].size.height() != source_rect.size.height) {
		RD::TextureFormat tf;
		tf.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
		tf.width = source_rect.size.width;
		tf.height = source_rect.size.height;
		tf.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_CAN_COPY_TO_BIT;
		tf.texture_type = RD::TEXTURE_TYPE_2D;
		history_textures[0].init(RD::get_singleton(), tf);
	}
}

void FilterChain::prepare_next_frame(RID p_source_texture, TextureSize p_source_size) {
	frame_count++;

	if (render_targets_need_resize) {
		resize_render_targets();
	}

	if (history_count > 0) {
		update_history();
		init_next_history_texture();
		Texture &texture = history_textures[0];

		RD::get_singleton()->texture_copy(
				p_source_texture, texture.rid,
				Vector3(source_rect.position.x, source_rect.position.y, 0),
				Vector3(0, 0, 0),
				Vector3(source_rect.size.width, source_rect.size.height, 1),
				0, 0, 0, 0);
	}
}

void FilterChain::update_buffers_for_passes() {
	UniformSetCacheRD *usc = UniformSetCacheRD::get_singleton();
	RD *rd = RD::get_singleton();

	for (uint32_t i = 0; i < passes_count; i++) {
		Pass &pass = passes[i];

		pass.frame_direction = uint32_t(frame_direction);
		pass.frame_count = frame_count;
		if (pass.frame_count_mod != 0) {
			pass.frame_count %= pass.frame_count_mod;
		}

		pass.bindings.ubo.update(rd);
		pass.bindings.push.update(rd);

		// swap feedback render targets
		if (pass.has_feedback) {
			SWAP(pass.render_target, pass.feedback_target);
		}

		if (pass.uniform_set.is_null() || pass.bindings.needs_texture_update) {
			for (auto &tb : pass.bindings.textures) {
				tb.uniform->set_id(1, *tb.texture);
			}
			pass.uniform_set = usc->get_cache_vec(pass.shader, 0, pass.bindings.uniforms);
		}
	}
}

void FilterChain::blit_texture(RenderingDevice::DrawListID p_draw_list, RID p_src) {
	RD *rd = RD::get_singleton();
	rd->draw_list_bind_render_pipeline(p_draw_list, pipeline_state.pipeline);
	rd->draw_list_set_push_constant(p_draw_list, &projection, sizeof(Projection));
	{
		RD::Uniform u(RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, 0, samplers[compiled::Filter::NEAREST][compiled::Wrap::EDGE]);
		u.append_id(p_src);
		pipeline_state.uniform_set = UniformSetCacheRD::get_singleton()->get_cache(pipeline_state.shader, 0, u);
	}
	rd->draw_list_bind_uniform_set(p_draw_list, pipeline_state.uniform_set, 0);
	rd->draw_list_draw(p_draw_list, false);
}

void FilterChain::render_final_pass(const RID p_target, const Size2 p_target_size) {
	RD *rd = RD::get_singleton();

	rd->draw_command_begin_label("RFX Final Pass");

	RD::DrawListID dl = rd->draw_list_begin(p_target);
	ERR_FAIL_COND_MSG(dl == RD::INVALID_ID, "Failed to create draw list for final pass.");

	rd->draw_list_set_viewport(dl, output_frame.viewport);
	rd->draw_list_bind_vertex_array(dl, pipeline_state.vertex_array);

	if (!has_shader || passes_count == 0) {
		RID src = history_textures[0].rid;
		blit_texture(dl, src);
	} else if (RID view = passes[last_pass_index].render_target.texture.rid; view.is_valid()) {
		blit_texture(dl, view);
	} else {
		Pass &pass = passes[last_pass_index];
		render_pass(pass, rd, dl);
	}

	rd->draw_list_end();

	rd->draw_command_end_label();
}

void FilterChain::render(
		const RID p_source, const Size2 p_source_size,
		const RID p_target, const Size2 p_target_size) {
	if (history_count == 0) {
		// No need to copy, set the sourceTexture to Original / OriginalHistory0 semantic.
		history_textures[0].rid = p_source;
		history_textures[0].size = p_source_size;
	}

	prepare_next_frame(p_source, p_source_size);
	update_buffers_for_passes();

	render_offscreen_passes();
	render_final_pass(p_target, p_target_size);

	if (history_count == 0) {
		// We don't own p_source, so clear it so it isn't freed by the FilterChain.
		history_textures[0].rid = RID();
	}
}

void FilterChain::render_offscreen_passes() {
	if (!has_shader || passes_count == 0) {
		return;
	}

	RD *rd = RD::get_singleton();

	bool last_pass_is_direct = passes[last_pass_index].render_target.is_valid() == false;
	uint32_t count = last_pass_is_direct ? passes_count - 1 : passes_count;
	for (uint32_t i = 0; i < count; i++) {
		Pass &pass = passes[i];
		{
			char label[16];
			int len = snprintf(label, sizeof(label), "RFX Pass %02d", i);
			RD::get_singleton()->draw_command_begin_label(Span<char>(label, len));
		}

		RD::DrawListID dl = rd->draw_list_begin(pass.render_target.frame_buffer);
		ERR_FAIL_COND_MSG(dl == RD::INVALID_ID, "Failed to create draw list for pass.");

		rd->draw_list_set_viewport(dl, pass.viewport);
		rd->draw_list_bind_vertex_array(dl, pipeline_state.vertex_array);
		render_pass(pass, rd, dl);
		rd->draw_list_end();

		rd->draw_command_end_label();
	}
}

void FilterChain::render_pass(FilterChain::Pass &p_pass, RD *p_rd, RenderingDevice::DrawListID p_draw_list) {
	p_rd->draw_list_bind_render_pipeline(p_draw_list, p_pass.pipeline);
	p_rd->draw_list_set_push_constant(p_draw_list, p_pass.bindings.push.binding.data.ptr(), p_pass.bindings.push.binding.data.size());
	p_rd->draw_list_bind_uniform_set(p_draw_list, p_pass.uniform_set, 0);
	p_rd->draw_list_draw(p_draw_list, false);
}

void FilterChain::init_samplers() {
	for (compiled::Wrap i = compiled::Wrap::BORDER; i < compiled::Wrap::MAX; ++i) {
		RD::SamplerState ss;
		switch (i) {
			case compiled::Wrap::BORDER:
				ss.repeat_u = RD::SAMPLER_REPEAT_MODE_CLAMP_TO_BORDER;
				break;
			case compiled::Wrap::EDGE:
				ss.repeat_u = RD::SAMPLER_REPEAT_MODE_CLAMP_TO_EDGE;
				break;
			case compiled::Wrap::REPEAT:
				ss.repeat_u = RD::SAMPLER_REPEAT_MODE_REPEAT;
				break;
			case compiled::Wrap::MIRRORED_REPEAT:
				ss.repeat_u = RD::SAMPLER_REPEAT_MODE_MIRRORED_REPEAT;
				break;
			case compiled::Wrap::MAX:
				// Should never happen
				break;
		}

		ss.repeat_v = ss.repeat_u;
		ss.repeat_w = ss.repeat_u;
		ss.min_filter = RD::SAMPLER_FILTER_LINEAR;
		ss.mag_filter = RD::SAMPLER_FILTER_LINEAR;
		ss.mip_filter = RD::SAMPLER_FILTER_LINEAR;

		samplers[compiled::Filter::LINEAR][i] = RD::get_singleton()->sampler_create(ss);

		ss.min_filter = RD::SAMPLER_FILTER_NEAREST;
		ss.mag_filter = RD::SAMPLER_FILTER_NEAREST;
		ss.mip_filter = RD::SAMPLER_FILTER_NEAREST;
		samplers[compiled::Filter::NEAREST][i] = RD::get_singleton()->sampler_create(ss);
	}
}

void FilterChain::set_rotation(float p_rotation) {
	rotation = p_rotation * 270.0f;
	projection.set_orthogonal(0, 1, 0, 1, -1, 1);
	Transform2D t(Math::deg_to_rad(rotation), Vector2());
	Projection rot;
	rot.columns[0][0] = t.columns[0][0];
	rot.columns[0][1] = t.columns[0][1];
	rot.columns[1][0] = t.columns[1][0];
	rot.columns[1][1] = t.columns[1][1];
	projection_no_rotate = rot * projection;
}

void FilterChain::set_default_filtering_linear(bool p_linear) {
	samplers[compiled::Filter::UNSPECIFIED] = samplers[p_linear ? compiled::Filter::LINEAR : compiled::Filter::NEAREST];
}

void FilterChain::update_history() {
	DEV_ASSERT(history_count > 0);

	if (history_needs_init) {
		init_history();
	} else {
		// shift history and move last texture into first position
		uint32_t last_index = history_count - 1;
		Texture last = history_textures[last_index];
		for (uint32_t k = last_index; k > 0; k--) {
			history_textures[k] = history_textures[k - 1];
		}
		history_textures[0] = last;
	}
}

void FilterChain::Pass::free_resources(RD *p_rd) {
	uniform_set = RID(); // managed by uniform set cache
	render_target.free(p_rd);
	feedback_target.free(p_rd);
	if (pipeline.is_valid()) {
		p_rd->free(pipeline);
		pipeline = RID();
	}
	if (shader.is_valid()) {
		p_rd->free(shader);
		shader = RID();
	}
	bindings.free(p_rd);
}

void FilterChain::free_resources(RD *p_rd) {
	for (uint32_t i = 0; i < passes_count; i++) {
		passes[i].free_resources(p_rd);
	}

	for (uint32_t i = 0; i < history_count; i++) {
		history_textures[i].free(p_rd);
	}

	RendererRD::TextureStorage *texture_storage = RendererRD::TextureStorage::get_singleton();
	for (uint32_t i = 0; i < textures_count; i++) {
		if (texture_rids[i].is_valid()) {
			texture_storage->free(texture_rids[i]);
		}
	}

	has_shader = false;
}

RID FilterChain::get_checker_texture() {
	if (checker_texture.is_null()) {
		Vector<uint8_t> checkerboard;
		checkerboard.resize(8 * 8 * 4);
		for (int i = 0; i < 8 * 8; i++) {
			checkerboard.set(i * 4 + 0, (i % 2) ? 0x00 : 0xff);
			checkerboard.set(i * 4 + 1, (i % 2) ? 0x00 : 0xff);
			checkerboard.set(i * 4 + 2, (i % 2) ? 0x00 : 0xff);
			checkerboard.set(i * 4 + 3, 0xff);
		}

		RD::TextureFormat tformat;
		tformat.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
		tformat.width = 8;
		tformat.height = 8;
		tformat.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_UPDATE_BIT;
		tformat.texture_type = RD::TEXTURE_TYPE_2D;

		checker_texture = RD::get_singleton()->texture_create(tformat, RD::TextureView(), { checkerboard });
	}
	return checker_texture;
}

RD::DataFormat to_data_format(compiled::OutputFormat p_format) {
	if (p_format.format.has_value()) {
		switch (p_format.format.value()) {
			case compiled::PixelFormat::r8Unorm:
				return RD::DATA_FORMAT_R8_UNORM;
			case compiled::PixelFormat::r8Uint:
				return RD::DATA_FORMAT_R8_UINT;
			case compiled::PixelFormat::r8Sint:
				return RD::DATA_FORMAT_R8_SINT;
			case compiled::PixelFormat::rg8Unorm:
				return RD::DATA_FORMAT_R8G8_UNORM;
			case compiled::PixelFormat::rg8Uint:
				return RD::DATA_FORMAT_R8G8_UINT;
			case compiled::PixelFormat::rg8Sint:
				return RD::DATA_FORMAT_R8G8_SINT;
			case compiled::PixelFormat::rgba8Unorm:
				return RD::DATA_FORMAT_R8G8B8A8_UNORM;
			case compiled::PixelFormat::rgba8Uint:
				return RD::DATA_FORMAT_R8G8B8A8_UINT;
			case compiled::PixelFormat::rgba8Sint:
				return RD::DATA_FORMAT_R8G8B8A8_SINT;
			case compiled::PixelFormat::rgba8Unorm_srgb:
				return RD::DATA_FORMAT_R8G8B8A8_SRGB;
			case compiled::PixelFormat::rgb10a2Unorm:
				return RD::DATA_FORMAT_A2B10G10R10_UNORM_PACK32;
			case compiled::PixelFormat::rgb10a2Uint:
				return RD::DATA_FORMAT_A2B10G10R10_UINT_PACK32;
			case compiled::PixelFormat::r16Uint:
				return RD::DATA_FORMAT_R16_UINT;
			case compiled::PixelFormat::r16Sint:
				return RD::DATA_FORMAT_R16_SINT;
			case compiled::PixelFormat::r16Float:
				return RD::DATA_FORMAT_R16_SFLOAT;
			case compiled::PixelFormat::rg16Uint:
				return RD::DATA_FORMAT_R16G16_UINT;
			case compiled::PixelFormat::rg16Sint:
				return RD::DATA_FORMAT_R16G16_SINT;
			case compiled::PixelFormat::rg16Float:
				return RD::DATA_FORMAT_R16G16_SFLOAT;
			case compiled::PixelFormat::rgba16Uint:
				return RD::DATA_FORMAT_R16G16B16A16_UINT;
			case compiled::PixelFormat::rgba16Sint:
				return RD::DATA_FORMAT_R16G16B16A16_SINT;
			case compiled::PixelFormat::rgba16Float:
				return RD::DATA_FORMAT_R16G16B16A16_SFLOAT;
			case compiled::PixelFormat::r32Uint:
				return RD::DATA_FORMAT_R32_UINT;
			case compiled::PixelFormat::r32Sint:
				return RD::DATA_FORMAT_R32_SINT;
			case compiled::PixelFormat::r32Float:
				return RD::DATA_FORMAT_R32_SFLOAT;
			case compiled::PixelFormat::rg32Uint:
				return RD::DATA_FORMAT_R32G32_UINT;
			case compiled::PixelFormat::rg32Sint:
				return RD::DATA_FORMAT_R32G32_SINT;
			case compiled::PixelFormat::rg32Float:
				return RD::DATA_FORMAT_R32G32_SFLOAT;
			case compiled::PixelFormat::rgba32Uint:
				return RD::DATA_FORMAT_R32G32B32A32_UINT;
			case compiled::PixelFormat::rgba32Sint:
				return RD::DATA_FORMAT_R32G32B32A32_SINT;
			case compiled::PixelFormat::rgba32Float:
				return RD::DATA_FORMAT_R32G32B32A32_SFLOAT;
			case compiled::PixelFormat::bgra8Unorm_srgb:
				return RD::DATA_FORMAT_B8G8R8A8_SRGB;
			case compiled::PixelFormat::bgra8Unorm:
				return RD::DATA_FORMAT_B8G8R8A8_UNORM;
		}
	}

	if (p_format.is_sRGB) {
		return RD::DATA_FORMAT_R8G8B8A8_SRGB;
	}

	if (p_format.is_float) {
		return RD::DATA_FORMAT_R16G16B16A16_SFLOAT;
	}

	return RD::DATA_FORMAT_R8G8B8A8_UNORM;
}

Error FilterChain::set_compiled_shader(const ShaderContainer &p_container) {
	RD *rd = RD::get_singleton();

	free_resources(rd);

	const compiled::Shader &shader = p_container.get_shader();

	passes_count = shader.passes.size();
	last_pass_index = passes_count - 1;

	parameters_count = shader.parameters.size();
	parameters_map.clear();
	for (uint32_t i = 0; i < shader.parameters.size(); i++) {
		parameters_map[shader.parameters[i].name.c_str()] = i;
	}
	for (uint32_t i = 0; i < shader.parameters.size(); i++) {
		parameters[i] = (float)shader.parameters[i].initial;
	}

	Pass &pass_first = passes[0];

	for (uint32_t pass_no = 0; pass_no < passes_count; pass_no++) {
		Pass &pass = passes[pass_no];
		pass.index = pass_no;

		shader::pass::Semantics sem;

		{
			Texture *first = &history_textures[0];

			sem.add_texture(&first->rid, &first->size, compiled::ShaderTextureSemantic::ORIGINAL);
			sem.add_texture(
					&first->rid, sizeof(Texture),
					&first->size, sizeof(Texture),
					compiled::ShaderTextureSemantic::ORIGINAL_HISTORY);
			if (pass_no == 0) {
				// The source texture for first pass is the original input texture
				sem.add_texture(&first->rid, &first->size, compiled::ShaderTextureSemantic::SOURCE);
			} else {
				// The source texture for passes [1, n) is the output of the previous pass
				sem.add_texture(
						&passes[pass_no - 1].render_target.texture.rid,
						&passes[pass_no - 1].render_target.texture.size,
						compiled::ShaderTextureSemantic::SOURCE);
			}
		}

		sem.add_texture(
				&pass_first.render_target.texture.rid, sizeof(Pass),
				&pass_first.render_target.texture.size, sizeof(Pass),
				compiled::ShaderTextureSemantic::PASS_OUTPUT);
		sem.add_texture(
				&pass_first.feedback_target.texture.rid, sizeof(Pass),
				&pass_first.feedback_target.texture.size, sizeof(Pass),
				compiled::ShaderTextureSemantic::PASS_FEEDBACK);

		{
			Texture *first = &textures[0];
			sem.add_texture(
					&first->rid, sizeof(Texture),
					&first->size, sizeof(Texture),
					compiled::ShaderTextureSemantic::USER);
		}

		if (pass_no == last_pass_index) {
			sem.add_uniform_data(&projection, compiled::ShaderBufferSemantic::MVP);
		} else {
			sem.add_uniform_data(&projection_no_rotate, compiled::ShaderBufferSemantic::MVP);
		}

		sem.add_uniform_data(&pass.render_target.texture.size, compiled::ShaderBufferSemantic::OUTPUT_SIZE);
		sem.add_uniform_data(&pass.frame_count, compiled::ShaderBufferSemantic::FRAME_COUNT);
		sem.add_uniform_data(&pass.frame_direction, compiled::ShaderBufferSemantic::FRAME_DIRECTION);

		sem.add_uniform_data(&output_frame.size, compiled::ShaderBufferSemantic::FINAL_VIEWPORT_SIZE);

		for (uint32_t i = 0; i < parameters_count; i++) {
			sem.add_float_parameter(&parameters[i], i);
		}

		const compiled::ShaderPass *shader_pass = &shader.passes[pass_no];
		init_bindings(pass, sem, shader_pass);
		pass.format = to_data_format(shader_pass->format);
		pass.frame_count_mod = shader_pass->frame_count_mod;

		// update scaling
		pass.scale_x = shader_pass->scale_x;
		pass.scale_y = shader_pass->scale_y;

		// create frame buffer for pass
		{
			Vector<RD::AttachmentFormat> attachments;
			{
				RD::AttachmentFormat af;
				af.format = pass.format;
				af.usage_flags = RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_CAN_COPY_TO_BIT;
				attachments.push_back(af);
			}
			pass.fb_format = rd->framebuffer_format_create(attachments);
		}

		// compile shader
		{
			Vector<RD::ShaderStageSPIRVData> stages;
			{
				String r_error;
				RD::ShaderStageSPIRVData stage;
				stage.spirv = rd->shader_compile_spirv_from_source(RD::SHADER_STAGE_VERTEX, shader_pass->vertex_source.c_str(), RD::SHADER_LANGUAGE_GLSL, &r_error);
				if (stage.spirv.is_empty()) {
					ERR_PRINT(vformat("Failed to compile vertex shader: %s", r_error));
					return ERR_CANT_CREATE;
				}
				stage.shader_stage = RD::SHADER_STAGE_VERTEX;
				stages.push_back(stage);
			}
			{
				String r_error;
				RD::ShaderStageSPIRVData stage;
				stage.spirv = rd->shader_compile_spirv_from_source(RD::SHADER_STAGE_FRAGMENT, shader_pass->fragment_source.c_str(), RD::SHADER_LANGUAGE_GLSL, &r_error);
				if (stage.spirv.is_empty()) {
					ERR_PRINT(vformat("Failed to compile fragment shader: %s", r_error));
					return ERR_CANT_CREATE;
				}
				stage.shader_stage = RD::SHADER_STAGE_FRAGMENT;
				stages.push_back(stage);
			}

			Vector<uint8_t> shader_data = rd->shader_compile_binary_from_spirv(stages, vformat("pass %d", pass_no));
			ERR_FAIL_COND_V_MSG(shader_data.is_empty(), ERR_CANT_CREATE, "Failed to compile shader");
			pass.shader = rd->shader_create_from_bytecode(shader_data);
			ERR_FAIL_COND_V_MSG(pass.shader.is_null(), ERR_CANT_CREATE, "Failed to create shader");
		}

		// create render pipeline
		pass.pipeline = rd->render_pipeline_create(
				pass.shader,
				pass.fb_format,
				pipeline_state.vert_format,
				RD::RENDER_PRIMITIVE_TRIANGLE_STRIPS,
				RD::PipelineRasterizationState(),
				RD::PipelineMultisampleState(),
				RD::PipelineDepthStencilState(),
				RD::PipelineColorBlendState::create_disabled());
		ERR_FAIL_COND_V_MSG(pass.pipeline.is_null(), ERR_CANT_CREATE, "Failed to create pipeline");
	}

	// remaining state
	history_count = shader.history_count;
	for (const compiled::ShaderPass &pass : shader.passes) {
		passes[pass.index].has_feedback = pass.is_feedback;
	}

	load_luts(p_container);

	has_shader = true;
	render_targets_need_resize = true;
	history_needs_init = true;

	return OK;
}

void FilterChain::load_luts(const ShaderContainer &p_container) {
	RendererRD::TextureStorage *texture_storage = RendererRD::TextureStorage::get_singleton();

	auto &images = p_container.get_shader().luts;
	textures_count = images.size();
	uint32_t i = 0;
	for (auto &lut : images) {
		RID texture;
		TextureSize size = TextureSize(8, 8); // default size of the checker texture
		if (auto data = p_container.get_lut_by_name(lut.name); data.is_empty()) {
			texture = get_checker_texture();
		} else {
			Ref<FileAccessMemory> memfile;
			memfile.instantiate();
			memfile->open_custom(data.ptr(), data.size());

			Ref<Image> img;
			img.instantiate();
			String path = lut.url.c_str();
			if (auto res = ImageLoader::load_image(path, img, memfile); res != OK) {
				print_error(vformat("Failed to load LUT: %s", path));
				texture = get_checker_texture();
			} else {
				texture_rids[i] = texture_storage->texture_allocate();
				texture_storage->texture_2d_initialize(texture_rids[i], img);
				texture = texture_storage->texture_get_rd_texture(texture_rids[i]);
				size = TextureSize(img->get_width(), img->get_height());
			}
		}

		textures[i].rid = texture;
		textures[i].size = size;
		i += 1;
	}
}

Error FilterChain::init_bindings(
		Pass &p_shader_pass,
		shader::pass::Semantics &p_sem,
		const compiled::ShaderPass *p_pass) {
	auto add_uniforms = [&](uint32_t p_size, shader::pass::BufferBinding &p_bind, const std::vector<compiled::BufferUniformDescriptor> &p_uniforms) {
		p_bind.data.resize((p_size + 0xf) & ~0xf); // round up to nearest 16 bytes

		for (compiled::BufferUniformDescriptor const &u : p_uniforms) {
			switch (u.semantic) {
				case compiled::ShaderBufferSemantic::FLOAT_PARAMETER: {
					if (auto param = p_sem.get_float_parameter(u.index.value()); param.has_value()) {
						p_bind.add_uniform(param->data, u.size, u.offset, u.name.c_str());
					} else {
						ERR_FAIL_MSG("Invalid float parameter index");
					}
				} break;

				case compiled::ShaderBufferSemantic::MVP:
				case compiled::ShaderBufferSemantic::OUTPUT_SIZE:
				case compiled::ShaderBufferSemantic::FINAL_VIEWPORT_SIZE:
				case compiled::ShaderBufferSemantic::FRAME_COUNT:
				case compiled::ShaderBufferSemantic::FRAME_DIRECTION: {
					if (auto uniform = p_sem.uniforms.getptr(u.semantic); uniform != nullptr) {
						p_bind.add_uniform(uniform->data, u.size, u.offset, u.name.c_str());
					} else {
						ERR_FAIL_MSG("Invalid uniform semantic");
					}
				} break;
				case compiled::ShaderBufferSemantic::ORIGINAL_SIZE:
				case compiled::ShaderBufferSemantic::SOURCE_SIZE:
				case compiled::ShaderBufferSemantic::ORIGINAL_HISTORY_SIZE:
				case compiled::ShaderBufferSemantic::PASS_OUTPUT_SIZE:
				case compiled::ShaderBufferSemantic::PASS_FEEDBACK_SIZE:
				case compiled::ShaderBufferSemantic::USER_SIZE: {
					if (auto texture = p_sem.texture_uniforms.getptr(u.semantic); texture != nullptr) {
						p_bind.add_uniform(
								(void *)((uintptr_t)texture->size + (u.index.value() * texture->stride)),
								u.size,
								u.offset,
								u.name.c_str());
					} else {
						ERR_FAIL_MSG("Invalid texture uniform semantic");
					}
				} break;
			}
		}
	};

	shader::pass::Bindings &bindings = p_shader_pass.bindings;

	uint32_t uniforms_size = p_pass->textures.size();
	if (p_pass->ubo.is_valid()) {
		uniforms_size += 1;
	}

	bindings.uniforms.resize(uniforms_size);
	RD::Uniform *uniforms_ptr = bindings.uniforms.ptr();

	if (p_pass->ubo.is_valid()) {
		add_uniforms(p_pass->ubo.size, bindings.ubo.binding, p_pass->ubo.uniforms);
		bindings.ubo.ubo_buffer = RD::get_singleton()->uniform_buffer_create(bindings.ubo.binding.data.size());
		*uniforms_ptr = RD::Uniform(RD::UNIFORM_TYPE_UNIFORM_BUFFER, p_pass->ubo.binding, bindings.ubo.ubo_buffer);
		uniforms_ptr++;
	}

	if (p_pass->push.is_valid()) {
		add_uniforms(p_pass->push.size, bindings.push.binding, p_pass->push.uniforms);
	}

	for (auto &t : p_pass->textures) {
		auto tex = p_sem.textures.getptr(t.semantic);
		bindings.needs_texture_update |= (t.semantic == compiled::ShaderTextureSemantic::ORIGINAL_HISTORY || t.semantic == compiled::ShaderTextureSemantic::PASS_FEEDBACK);

		RD::Uniform *u = uniforms_ptr;
		uniforms_ptr++;
		u->uniform_type = RD::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE;
		u->binding = t.binding;
		u->append_id(samplers[t.filter][t.wrap]);
		u->append_id(RID()); // placeholder for texture
		bindings.add_texture(u, (RID *)((uintptr_t)tex->texture + (t.index * tex->stride)), t.name.c_str());
	}

	return OK;
}

void FilterChain::init_history() {
	RD *rd = RD::get_singleton();

	RD::TextureFormat tf;
	tf.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
	tf.width = source_rect.size.width;
	tf.height = source_rect.size.height;
	tf.usage_bits = RD::TEXTURE_USAGE_SAMPLING_BIT | RD::TEXTURE_USAGE_STORAGE_BIT | RD::TEXTURE_USAGE_CAN_COPY_TO_BIT;
	tf.texture_type = RD::TEXTURE_TYPE_2D;

	for (uint32_t k = 0; k < history_count; k++) {
		history_textures[k].init(rd, tf);
	}

	history_needs_init = false;
}

FilterChain::FilterChain() {
	RD *rd = RD::get_singleton();

	// initialize shader and pipeline
	{
		Vector<String> defines;
		defines.push_back("");
		final_blit_shader.initialize(defines);

		shader_version = final_blit_shader.version_create();
		pipeline_state.shader = final_blit_shader.version_get_shader(shader_version, 0);

		{
			Vector<RD::AttachmentFormat> attachments;
			{
				RD::AttachmentFormat att;
				att.format = RD::DATA_FORMAT_R8G8B8A8_UNORM;
				att.usage_flags = RD::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | RD::TEXTURE_USAGE_SAMPLING_BIT;
				attachments.push_back(att);
			}

			pipeline_state.fb_format = rd->framebuffer_format_create(attachments);
		}

		{
			Vector<RD::VertexAttribute> attrs;

			{
				RD::VertexAttribute va;
				va.location = 0;
				va.offset = offsetof(Vertex, position);
				va.format = RD::DATA_FORMAT_R32G32B32A32_SFLOAT;
				va.stride = sizeof(Vertex);
				attrs.push_back(va);
			}
			{
				RD::VertexAttribute va;
				va.location = 1;
				va.offset = offsetof(Vertex, tex_coord);
				va.format = RD::DATA_FORMAT_R32G32_SFLOAT;
				va.stride = sizeof(Vertex);
				attrs.push_back(va);
			}

			pipeline_state.vert_format = rd->vertex_format_create(attrs);
		}
		pipeline_state.pipeline = rd->render_pipeline_create(
				pipeline_state.shader,
				pipeline_state.fb_format, pipeline_state.vert_format,
				RD::RENDER_PRIMITIVE_TRIANGLE_STRIPS, {}, {}, {}, RD::PipelineColorBlendState::create_disabled());
	}

	// vertex buffers
	{
		Vector<uint8_t> data;
		data.resize(sizeof(vertex));
		memcpy(data.ptrw(), vertex, sizeof(vertex));
		pipeline_state.vertex_buffer = rd->vertex_buffer_create(sizeof(vertex), data);
		pipeline_state.vertex_array = rd->vertex_array_create(4, pipeline_state.vert_format, { pipeline_state.vertex_buffer, pipeline_state.vertex_buffer });
	}

	memset(textures, 0, sizeof(textures));

	init_samplers();
	set_rotation(0);
	set_default_filtering_linear(false);
}

FilterChain::~FilterChain() {
	RD *rd = RD::get_singleton();

	free_resources(rd);

	// Don't free index 0, as that is either a reference to linear or nearest, and is the default,
	// when the filter is unspecified.
	for (compiled::Filter i = compiled::Filter::LINEAR; i < compiled::Filter::MAX; ++i) {
		for (compiled::Wrap j = compiled::Wrap::BORDER; j < compiled::Wrap::MAX; ++j) {
			if (samplers[i][j].is_valid()) {
				rd->free(samplers[i][j]);
			}
		}
	}

	if (pipeline_state.uniform_set.is_valid()) {
		rd->free(pipeline_state.uniform_set);
	}
	rd->free(pipeline_state.vertex_array);
	rd->free(pipeline_state.vertex_buffer);
	rd->free(pipeline_state.pipeline);
	final_blit_shader.version_free(shader_version);
	if (checker_texture.is_valid()) {
		rd->free(checker_texture);
	}
}
