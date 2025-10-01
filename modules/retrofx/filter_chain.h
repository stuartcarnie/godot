//
// Created by Stuart Carnie on 7/8/2024.
//

#pragma once

#include "constants.h"
#include "shader_container.h"
#include "shader_pass_bindings.h"
#include "shader_pass_semantics.h"

#include "modules/retrofx/final_blit.glsl.gen.h"
#include "servers/rendering/rendering_device.h"

class FilterChain {
	struct SamplerWrapArray {
		RID &operator[](compiled::Wrap p_i) {
			return d[to_int(p_i)];
		}

	private:
		RID d[to_int(compiled::Wrap::MAX)];
	};

	struct SamplerFilterArray {
		SamplerWrapArray &operator[](compiled::Filter p_i) {
			return d[to_int(p_i)];
		}

	private:
		SamplerWrapArray d[to_int(compiled::Filter::MAX)];
	};

	struct [[nodiscard]] TextureSize {
		TextureSize() = default;
		TextureSize(float p_width, float p_height) {
			d = Vector4(p_width, p_height, 1.0f / p_width, 1.0f / p_height);
		}
		TextureSize(Size2 p_size) :
				TextureSize(p_size.width, p_size.height) {}

		_FORCE_INLINE_ float width() const { return d.x; }
		_FORCE_INLINE_ float height() const { return d.y; }

	private:
		Vector4 d;
	};

	struct Texture {
		RID rid;
		TextureSize size;

		_FORCE_INLINE_ void init(RD *p_rd, const RD::TextureFormat &p_format, const bool p_clear = true) {
			if (rid.is_valid()) {
				p_rd->free_rid(rid);
			}
			rid = p_rd->texture_create(p_format, RD::TextureView());
			size = TextureSize(p_format.width, p_format.height);
			p_rd->texture_clear(rid, Color(0, 0, 0), 0, 1, 0, 1);
		}

		_FORCE_INLINE_ void free(RD *p_rd) {
			if (rid.is_valid()) {
				p_rd->free_rid(rid);
				rid = RID();
			}
		}
	};

	struct RenderTexture {
		Texture texture;
		RID frame_buffer;

		_FORCE_INLINE_ float width() const { return texture.size.width(); }
		_FORCE_INLINE_ float height() const { return texture.size.height(); }

		void init(RD *p_rd, const RD::TextureFormat &p_format, const RD::FramebufferFormatID &p_fb_format) {
			free(p_rd);
			texture.init(p_rd, p_format);
			frame_buffer = p_rd->framebuffer_create({ texture.rid }, p_fb_format);
			p_rd->framebuffer_set_invalidation_callback(frame_buffer, [](void *p_ud) {
				RenderTexture *rt = static_cast<RenderTexture *>(p_ud);
				rt->frame_buffer = RID(); }, this);
		}

		_FORCE_INLINE_ void free(RD *p_rd) {
			if (frame_buffer.is_valid()) {
				p_rd->free_rid(frame_buffer);
				frame_buffer = RID();
			}
			texture.free(p_rd);
		}

		_FORCE_INLINE_ bool is_valid() const {
			return frame_buffer.is_valid() && texture.rid.is_valid();
		}
	};

	struct Pass {
		/// The index of the pass in the shader graph.
		uint32_t index = 0;
		/// The output format of the pass.
		///
		/// If the value is <code>RD::DATA_FORMAT_MAX</code>, the format chosen
		/// automatically.
		RD::DataFormat format = RD::DATA_FORMAT_MAX;
		RenderTexture render_target;
		RenderTexture feedback_target;
		uint32_t frame_count = 0;
		uint32_t frame_count_mod = 0;
		int32_t frame_direction = 0;
		shader::pass::Bindings bindings;
		Rect2i viewport;
		RD::FramebufferFormatID fb_format;
		RID shader;
		RID pipeline;
		RID uniform_set;
		bool has_feedback = false;
		optional<compiled::Scale> scale_x;
		optional<compiled::Scale> scale_y;

		bool is_scaled() const {
			return scale_x.has_value() || scale_y.has_value();
		}

		Size2i get_output_size(Size2i p_viewport, Size2i p_source) const;

		void free_resources(RD *p_rd);
	};

	struct Vertex {
		Vector4 position;
		Vector2 tex_coord;
	};

	struct OutputFrame {
		Rect2 viewport;
		TextureSize size;
	};

	static Vertex vertex[4];

	SamplerFilterArray samplers;

	bool has_shader = false;

	uint32_t frame_count = 0;
	uint32_t last_pass_index = 0;

	/// The OriginalHistory<N> semantic
	Texture history_textures[MAX_FRAME_HISTORY + 1];
	uint32_t history_count = 0;

	OutputFrame output_frame;

	Pass passes[MAX_SHADER_PASSES];
	uint32_t passes_count = 0;
	RID texture_rids[MAX_TEXTURES];
	Texture textures[MAX_TEXTURES];
	uint32_t textures_count = 0;

	bool render_targets_need_resize = true;
	bool history_needs_init = false;

	Rect2 source_rect;
	Size2 aspect_size;
	Rect2 output_bounds;
	Size2 drawable_size;

	int frame_direction = 1;

	/// Render target pipeline state
	FinalBlitShaderRD final_blit_shader;
	RID shader_version;
	struct {
		RID shader;
		RID pipeline;
		RD::FramebufferFormatID fb_format;
		RD::VertexFormatID vert_format;
		RID vertex_buffer;
		RID vertex_array;
		RID uniform_set;
	} pipeline_state;

	float rotation = 0;

	Projection projection;
	Projection projection_no_rotate;

	float parameters[MAX_PARAMETERS];
	uint32_t parameters_count = 0;
	HashMap<String, uint32_t> parameters_map;

	/// Used as a fallback image when a look-up texture cannot be loaded.
	RID checker_texture;
	RID get_checker_texture();

	void update_history();

	void init_samplers();
	void init_history();

	static Rect2 fit_aspect_rect_into_rect(Size2 p_aspect, Size2 p_size);
	void resize();
	void resize_render_targets();
	void init_next_history_texture();
	void prepare_next_frame(RID p_source_texture, TextureSize p_source_size);
	void update_buffers_for_passes();
	void render_pass(Pass &p_pass, RD *p_rd, RenderingDevice::DrawListID p_draw_list);
	void blit_texture(RenderingDevice::DrawListID p_draw_list, RID p_src);

	void load_luts(const ShaderContainer &p_container);
	Error init_bindings(Pass &p_shader_pass, shader::pass::Semantics &p_sem, const compiled::ShaderPass *p_pass);
	void free_resources(RD *p_rd);

public:
	Error set_compiled_shader(const ShaderContainer &p_container);

	void set_rotation(float p_rotation);
	float get_rotation() const { return rotation; }

	void set_source_rect(Rect2 p_rect, Size2 p_aspect);
	Rect2 get_source_rect() const { return source_rect; }

	void set_drawable_size(Size2 p_size);
	Size2 get_drawable_size() const { return drawable_size; }

	Rect2 get_output_bounds() const { return output_bounds; }

	void set_parameter_value(uint32_t p_index, double p_value);
	void set_parameter_value(const String &p_name, double p_value);
	bool get_parameter_value(uint32_t p_index, double &r_value) const;
	bool get_parameter_value(const String &p_name, double &r_value) const;

	void render(const RID p_source, const Size2 p_source_size, const RID p_target, const Size2 p_target_size);
	void render_offscreen_passes();
	void render_final_pass(const RID p_target, const Size2 p_target_size);

	/// Sets the default filtering mode when a shader pass leaves the value unspecified.
	///
	/// @param p_linear <code>true</code> to default to linear filtering; otherwise, default to nearest.
	void set_default_filtering_linear(bool p_linear);

	_FORCE_INLINE_ bool has_shader_loaded() const { return has_shader; }

	FilterChain();
	~FilterChain();
};
