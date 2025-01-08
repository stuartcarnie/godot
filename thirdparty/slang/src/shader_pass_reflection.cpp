//
// Created by Stuart Carnie on 13/8/2024.
//

#include "shader_pass_reflection.h"
#include <sstream>

namespace slang {

error::ErrorOpt ShaderPassReflection::set_offset(int p_offset, int vec_size, int p_index, std::string const &p_name, bool p_ubo) {
	ShaderBufferSemanticMetaRef sem;
	if (auto it = float_parameters.find(p_index); it != float_parameters.end()) {
		sem = it->second;
	} else {
		sem = ShaderBufferSemanticMeta::create(p_index, p_name);
		float_parameters[p_index] = sem;
	}

	if (sem->number_of_components != vec_size && (sem->ubo_offset.has_value() || sem->push_offset.has_value())) {
		std::ostringstream msg;
		msg << "vertex and fragment shaders have different data type sizes for same parameter #" << p_index << " (" << sem->number_of_components << " / " << vec_size << ")";
		return error::Error::failed(msg.str());
	}

	if (p_ubo) {
		if (sem->ubo_offset.has_value() && sem->ubo_offset.value() != p_offset) {
			std::ostringstream msg;
			msg << "vertex and fragment shaders have different offsets for same parameter #" << p_index << " (" << sem->ubo_offset.value() << " / " << p_offset << ")";
			return error::Error::failed(msg.str());
		}
		sem->ubo_offset = p_offset;
	} else {
		if (sem->push_offset.has_value() && sem->push_offset.value() != p_offset) {
			std::ostringstream msg;
			msg << "vertex and fragment shaders have different offsets for same parameter #" << p_index << " (" << sem->push_offset.value() << " / " << p_offset << ")";
			return error::Error::failed(msg.str());
		}
		sem->push_offset = p_offset;
	}

	sem->number_of_components = vec_size;

	return {};
}

error::ErrorOpt ShaderPassReflection::set_offset(int p_offset, int vec_size, compiled::ShaderBufferSemantic p_semantic, bool p_ubo) {
	ShaderBufferSemanticMetaRef sem;
	if (auto it = semantics.find(p_semantic); it != semantics.end()) {
		sem = it->second;
	} else {
		// invalid semantic, we should return an error
		return {};
	}

	if (sem->number_of_components != vec_size && (sem->ubo_offset.has_value() || sem->push_offset.has_value())) {
		std::ostringstream msg;
		msg << "vertex and fragment shaders have different data type sizes for same semantic " << compiled::to_string(p_semantic) << " (" << sem->number_of_components << " / " << vec_size << ")";
		return error::Error::failed(msg.str());
	}

	if (p_ubo) {
		if (sem->ubo_offset.has_value() && sem->ubo_offset.value() != p_offset) {
			std::ostringstream msg;
			msg << "vertex and fragment shaders have different offsets for same semantic " << compiled::to_string(p_semantic) << " (" << sem->ubo_offset.value() << " / " << p_offset << ")";
			return error::Error::failed(msg.str());
		}
		sem->ubo_offset = p_offset;
	} else {
		if (sem->push_offset.has_value() && sem->push_offset.value() != p_offset) {
			std::ostringstream msg;
			msg << "vertex and fragment shaders have different offsets for same semantic " << compiled::to_string(p_semantic) << " (" << sem->push_offset.value() << " / " << p_offset << ")";
			return error::Error::failed(msg.str());
		}
		sem->push_offset = p_offset;
	}

	sem->number_of_components = vec_size;

	return {};
}

error::ErrorOpt ShaderPassReflection::set_offset(int p_offset, compiled::ShaderBufferSemantic p_semantic, int p_index, std::string const &p_name, bool p_ubo) {
	std::map<int, ShaderBufferSemanticMetaRef> *map;
	if (auto it = texture_uniforms.find(p_semantic); it != texture_uniforms.end()) {
		map = &it->second;
	} else {
		// invalid semantic, we should return an error
		return {};
	}

	ShaderBufferSemanticMetaRef sem;
	if (auto it = map->find(p_index); it != map->end()) {
		sem = it->second;
	} else {
		sem = ShaderBufferSemanticMeta::create(p_index, p_name);
		map->operator[](p_index) = sem;
	}

	if (p_ubo) {
		if (sem->ubo_offset.has_value() && sem->ubo_offset.value() != p_offset) {
			std::ostringstream msg;
			msg << "vertex and fragment shaders have different offsets for same semantic " << compiled::to_string(p_semantic) << " (" << sem->ubo_offset.value() << " / " << p_offset << ")";
			return error::Error::failed(msg.str());
		}
		sem->ubo_offset = p_offset;
	} else {
		if (sem->push_offset.has_value() && sem->push_offset.value() != p_offset) {
			std::ostringstream msg;
			msg << "vertex and fragment shaders have different offsets for same semantic " << compiled::to_string(p_semantic) << " (" << sem->push_offset.value() << " / " << p_offset << ")";
			return error::Error::failed(msg.str());
		}
		sem->push_offset = p_offset;
	}

	return {};
}

error::ErrorOpt ShaderPassReflection::set_binding(int p_binding, ShaderPassReflection::STS p_semantic, int p_index, std::string const &p_name) {
	std::map<int, ShaderTextureSemanticMetaRef> *map;
	if (auto it = textures.find(p_semantic); it != textures.end()) {
		map = &it->second;
	} else {
		// invalid semantic, we should return an error
		return {};
	}

	ShaderTextureSemanticMetaRef sem;
	if (auto it = map->find(p_index); it != map->end()) {
		sem = it->second;
	} else {
		sem = ShaderTextureSemanticMeta::create(p_index, p_name);
		map->operator[](p_index) = sem;
	}

	sem->binding = p_binding;

	return {};
}

template <typename K, typename V, typename Compare>
std::vector<V> sort_map_values(const std::map<K, V> &input_map, Compare comp) {
	// Extract values into a vector
	std::vector<V> values;
	for (const auto &pair : input_map) {
		values.push_back(pair.second);
	}

	// Sort the vector using the provided comparator
	std::sort(values.begin(), values.end(), comp);

	return values;
}

void ShaderPassReflection::to_stream(std::ostream &oss) const {
	oss << "Pass #" << pass_number << std::endl
		<< "  → textures:" << std::endl;

	for (STS sem : compiled::texture_semantics()) {
		if (auto it = textures.find(sem); it != textures.end()) {
			auto sorted = sort_map_values(it->second, [](const auto &a, const auto &b) {
				return a->index < b->index;
			});
			for (auto &meta : sorted) {
				if (meta->binding.has_value()) {
					oss << "      " << compiled::to_string(sem) << " (" << meta->name << " #" << meta->index << ")" << std::endl;
				}
			}
		}
	}

	oss << std::endl;
	oss << "  → Uniforms (vertex: "
		<< (ubo.is_vert() ? "YES" : "NO")
		<< ", fragment "
		<< (ubo.is_frag() ? "YES" : "NO")
		<< "):" << std::endl;

	for (SBS sem : compiled::buffer_semantics()) {
		if (auto it = semantics.find(sem); it != semantics.end()) {
			if (it->second->ubo_offset.has_value()) {
				oss << "      " << compiled::to_string(sem) << " (offset: " << it->second->ubo_offset.value() << ")" << std::endl;
			}
		}
	}

	for (SBS sem : compiled::buffer_semantics()) {
		if (auto it = texture_uniforms.find(sem); it != texture_uniforms.end()) {
			auto sorted = sort_map_values(it->second, [](const auto &a, const auto &b) {
				return a->index < b->index;
			});
			for (auto &meta : sorted) {
				if (meta->ubo_offset.has_value()) {
					oss << "      " << meta->name << " (#" << meta->index << ") (offset: " << meta->ubo_offset.value() << ")" << std::endl;
				}
			}
		}
	}

	oss << std::endl
		<< "  → Push (vertex: "
		<< (push.is_vert() ? "YES" : "NO")
		<< ", fragment "
		<< (push.is_frag() ? "YES" : "NO")
		<< "):" << std::endl;

	for (SBS sem : compiled::buffer_semantics()) {
		if (auto it = semantics.find(sem); it != semantics.end()) {
			if (it->second->push_offset.has_value()) {
				oss << "      " << compiled::to_string(sem) << " (offset: " << it->second->push_offset.value() << ")" << std::endl;
			}
		}
	}

	for (SBS sem : compiled::buffer_semantics()) {
		if (auto it = texture_uniforms.find(sem); it != texture_uniforms.end()) {
			auto sorted = sort_map_values(it->second, [](const auto &a, const auto &b) {
				return a->index < b->index;
			});
			for (auto &meta : sorted) {
				if (meta->push_offset.has_value()) {
					oss << "      " << meta->name << " (#" << meta->index << ") (offset: " << meta->push_offset.value() << ")\n";
				}
			}
		}
	}

	oss << std::endl
		<< "  → Parameters:" << std::endl;

	auto sorted = sort_map_values(float_parameters, [](const auto &a, const auto &b) {
		return a->index < b->index;
	});
	for (auto &meta : sorted) {
		if (meta->ubo_offset.has_value()) {
			oss << "      UBO  " << meta->name << " #" << meta->index << " (offset: " << meta->ubo_offset.value() << ")" << std::endl;
		}
		if (meta->push_offset.has_value()) {
			oss << "      PUSH " << meta->name << " #" << meta->index << " (offset: " << meta->push_offset.value() << ")" << std::endl;
		}
	}

	oss << std::endl;
}

} // namespace slang
