//
// Metal shader cache dump tool.
//
// Reads Godot shader cache files (.cache) and prints the MSL source for each
// shader stage. The file format is:
//
//   Outer: "GDSC" + version(4) + variant_count + {size, bytes}...
//   Inner: RenderingShaderContainer with Metal-specific extra data.
//

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#include "thirdparty/cxxopts/include/cxxopts.hpp"
#pragma clang diagnostic pop
#include "types.h"
#include "zstd.h"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <optional>
#include <set>
#include <vector>

struct ShaderEntry {
	std::string name;
	uint32_t variant;
	std::string type;
	std::string source;
};

static std::optional<ShaderStage> stage_from_string(const std::string &p_str) {
	if (strncasecmp(p_str.c_str(), "vert", 4) == 0) {
		return SHADER_STAGE_VERTEX;
	}
	if (strncasecmp(p_str.c_str(), "frag", 4) == 0) {
		return SHADER_STAGE_FRAGMENT;
	}
	if (strncasecmp(p_str.c_str(), "comp", 4) == 0) {
		return SHADER_STAGE_COMPUTE;
	}
	return std::nullopt;
}

static const char *stage_to_string(uint32_t p_stage) {
	switch (p_stage) {
		case SHADER_STAGE_VERTEX:
			return "vertex";
		case SHADER_STAGE_FRAGMENT:
			return "fragment";
		case SHADER_STAGE_COMPUTE:
			return "compute";
		default:
			return "other";
	}
}

static std::string json_escape(const std::string &p_str) {
	std::string out;
	out.reserve(p_str.size() + 32);
	for (char c : p_str) {
		switch (c) {
			case '"':
				out += "\\\"";
				break;
			case '\\':
				out += "\\\\";
				break;
			case '\n':
				out += "\\n";
				break;
			case '\r':
				out += "\\r";
				break;
			case '\t':
				out += "\\t";
				break;
			default:
				out += c;
				break;
		}
	}
	return out;
}

// Parse a RenderingShaderContainer blob and collect shader entries.
static bool parse_container(const uint8_t *p_data, size_t p_length, uint32_t p_variant,
		const std::set<ShaderStage> &p_stages, std::vector<ShaderEntry> &r_entries) {
	constexpr size_t alignment = sizeof(uint32_t);
	size_t pos = 0;

#define CHECK(p_size, p_what)                                   \
	if (pos + (p_size) > p_length) {                            \
		fprintf(stderr, "short buffer reading %s\n", (p_what)); \
		return false;                                           \
	}

	// Container header.
	CHECK(sizeof(ContainerHeader), "container header");
	const ContainerHeader &hdr = *(const ContainerHeader *)(p_data + pos);
	pos += sizeof(ContainerHeader);

	if (hdr.magic_number != CONTAINER_MAGIC) {
		fprintf(stderr, "invalid container magic: 0x%08x\n", hdr.magic_number);
		return false;
	}
	if (hdr.format != METAL_FORMAT) {
		fprintf(stderr, "not a Metal shader container (format=0x%08x)\n", hdr.format);
		return false;
	}

	// Reflection data.
	CHECK(sizeof(ReflectionData), "reflection data");
	const ReflectionData &refl = *(const ReflectionData *)(p_data + pos);
	pos += sizeof(ReflectionData);

	// Metal header extra data.
	CHECK(sizeof(MetalHeaderData), "metal header data");
	pos += sizeof(MetalHeaderData);

	// Shader name.
	std::string shader_name;
	if (refl.shader_name_len > 0) {
		CHECK(refl.shader_name_len, "shader name");
		shader_name.assign((const char *)(p_data + pos), refl.shader_name_len);
		size_t colon = shader_name.rfind(':');
		if (colon != std::string::npos) {
			shader_name.erase(colon);
		}
		pos = aligned_to(pos + refl.shader_name_len, alignment);
	}

	// Skip uniform sets.
	for (uint32_t i = 0; i < refl.set_count; i++) {
		CHECK(sizeof(uint32_t), "uniform set count");
		uint32_t uniforms_count = *(const uint32_t *)(p_data + pos);
		pos += sizeof(uint32_t);
		size_t set_size = uniforms_count * (sizeof(ReflectionBindingData) + sizeof(MetalUniformData));
		CHECK(set_size, "uniform set data");
		pos += set_size;
	}

	// Skip specialization constants.
	{
		size_t spec_size = refl.specialization_constants_count * sizeof(ReflectionSpecializationData);
		CHECK(spec_size, "specialization constants");
		pos += spec_size;
	}

	// Skip stage list.
	if (refl.stage_count > 0) {
		size_t stages_size = refl.stage_count * sizeof(uint32_t);
		CHECK(stages_size, "stage list");
		pos += stages_size;
	}

	// Read shaders.
	for (uint32_t i = 0; i < hdr.shader_count; i++) {
		CHECK(sizeof(ShaderHeader), "shader header");
		const ShaderHeader &shader_hdr = *(const ShaderHeader *)(p_data + pos);
		pos += sizeof(ShaderHeader);

		CHECK(shader_hdr.code_compressed_size, "shader code");
		const uint8_t *compressed = p_data + pos;
		pos = aligned_to(pos + shader_hdr.code_compressed_size, alignment);

		// Metal stage extra data.
		CHECK(sizeof(MetalStageData), "metal stage data");
		const MetalStageData &stage_data = *(const MetalStageData *)(p_data + pos);
		pos += sizeof(MetalStageData);

		// Filter by stage.
		ShaderStage stage = (ShaderStage)shader_hdr.shader_stage;
		if (!p_stages.empty() && p_stages.find(stage) == p_stages.end()) {
			continue;
		}

		// Decompress.
		std::vector<uint8_t> decompressed(shader_hdr.code_decompressed_size);
		if (shader_hdr.code_compression_flags & COMPRESSION_FLAG_ZSTD) {
			size_t ret = ZSTD_decompress(decompressed.data(), decompressed.size(),
					compressed, shader_hdr.code_compressed_size);
			if (ZSTD_isError(ret)) {
				fprintf(stderr, "zstd decompression failed: %s\n", ZSTD_getErrorName(ret));
				return false;
			}
		} else {
			memcpy(decompressed.data(), compressed,
					std::min((size_t)shader_hdr.code_compressed_size, decompressed.size()));
		}

		uint32_t source_size = stage_data.source_size;
		if (source_size > decompressed.size()) {
			source_size = decompressed.size();
		}

		ShaderEntry entry;
		entry.name = shader_name;
		entry.variant = p_variant;
		entry.type = stage_to_string(shader_hdr.shader_stage);
		entry.source.assign((const char *)decompressed.data(), source_size);
		r_entries.push_back(std::move(entry));
	}

#undef CHECK
	return true;
}

int main(int argc, char *argv[]) {
	cxxopts::Options options("metal-tools", "Dump Metal shader sources from Godot shader cache files");

	// clang-format off
	options.add_options()
			("s,stages", "Shader stage(s) to print [vertex,fragment,compute]", cxxopts::value<std::vector<std::string>>())
			("j,json", "Output as JSON")
("filenames", "The filename(s) to process", cxxopts::value<std::vector<std::string>>())
			;
	// clang-format on

	options.parse_positional({ "filenames" });

	cxxopts::ParseResult result;
	try {
		result = options.parse(argc, argv);
	} catch (const cxxopts::exceptions::exception &e) {
		fprintf(stderr, "%s\n\n", e.what());
		std::cerr << options.help() << std::endl;
		return 1;
	}

	bool json_output = result.count("json") > 0;
	std::set<ShaderStage> stages;
	if (result.count("stages")) {
		for (const std::string &s : result["stages"].as<std::vector<std::string>>()) {
			std::optional<ShaderStage> stage = stage_from_string(s);
			if (stage) {
				stages.insert(stage.value());
			}
		}
	}

	if (result.count("filenames") == 0) {
		std::cout << options.help() << std::endl;
		return 1;
	}

	std::vector<std::string> filenames = result["filenames"].as<std::vector<std::string>>();
	std::vector<ShaderEntry> entries;

	for (const std::string &filename : filenames) {
		FILE *file = fopen(filename.c_str(), "rb");
		if (file == nullptr) {
			perror("Error opening file");
			return 1;
		}

		fseek(file, 0, SEEK_END);
		long filesize = ftell(file);
		fseek(file, 0, SEEK_SET);

		std::vector<uint8_t> buffer(filesize);
		fread(buffer.data(), 1, filesize, file);
		fclose(file);

		const uint8_t *ptr = buffer.data();
		const uint8_t *end = ptr + filesize;

		// Outer GDSC wrapper.
		if (filesize < 12) {
			fprintf(stderr, "%s: file too small\n", filename.c_str());
			return 1;
		}

		if (memcmp(ptr, shader_file_header, 4) != 0) {
			fprintf(stderr, "%s: invalid header\n", filename.c_str());
			return 1;
		}
		ptr += 4;

		uint32_t version = *(const uint32_t *)ptr;
		ptr += 4;
		if (version != cache_file_version) {
			fprintf(stderr, "%s: unsupported version %u (expected %u)\n",
					filename.c_str(), version, cache_file_version);
			return 1;
		}

		uint32_t variant_count = *(const uint32_t *)ptr;
		ptr += 4;

		for (uint32_t i = 0; i < variant_count; i++) {
			if (ptr + 4 > end) {
				fprintf(stderr, "unexpected end of file in variant %u\n", i);
				return 1;
			}
			uint32_t variant_size = *(const uint32_t *)ptr;
			ptr += 4;

			if (variant_size == 0) {
				continue;
			}

			if (ptr + variant_size > end) {
				fprintf(stderr, "variant %u: size %u exceeds file\n", i, variant_size);
				return 1;
			}

			parse_container(ptr, variant_size, i, stages, entries);
			ptr += variant_size;
		}
	}

	if (!stages.empty() && entries.empty()) {
		fprintf(stderr, "no shaders matched the requested stage(s)\n");
		return 1;
	}

	if (json_output) {
		printf("[\n");
		for (size_t i = 0; i < entries.size(); i++) {
			const ShaderEntry &e = entries[i];
			printf("  {\n");
			printf("    \"name\": \"%s\",\n", json_escape(e.name).c_str());
			printf("    \"variant\": %u,\n", e.variant);
			printf("    \"type\": \"%s\",\n", e.type.c_str());
			printf("    \"source\": \"%s\"\n", json_escape(e.source).c_str());
			printf("  }%s\n", (i + 1 < entries.size()) ? "," : "");
		}
		printf("]\n");
	} else {
		for (const ShaderEntry &e : entries) {
			if (filenames.size() > 1) {
				printf("file: variant=%u\n", e.variant);
			}
			printf("%s: type=%s\n", e.name.c_str(), e.type.c_str());
			fwrite(e.source.data(), 1, e.source.size(), stdout);
			printf("\n");
		}
	}

	return 0;
}
