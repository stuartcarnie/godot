//
// Created by Stuart Carnie on 26/10/2023.
//

#include "thirdparty/cxxopts/include/cxxopts.hpp"
#include "types.h"
#include "zstd.h"

#include <iostream>
#include <vector>
#include <set>

std::optional<RD::ShaderStage> stage_from_string(std::string s) {
	if (strncasecmp(s.c_str(), "vert", 4) == 0) {
		return RD::ShaderStage::SHADER_STAGE_VERTEX;
	}

	if (strncasecmp(s.c_str(), "frag", 4) == 0) {
		return RD::ShaderStage::SHADER_STAGE_FRAGMENT;
	}

	if (strncasecmp(s.c_str(), "comp", 4) == 0) {
		return RD::ShaderStage::SHADER_STAGE_COMPUTE;
	}

	return std::nullopt;
}

char const *stage_to_string(RD::ShaderStage stage) {
	switch (stage) {
		case RD::SHADER_STAGE_COMPUTE:
			return "compute";
		case RD::SHADER_STAGE_FRAGMENT:
			return "fragment";
		case RD::SHADER_STAGE_VERTEX:
			return "vertex";
		default:
			return "other";
	}
}

int main(int argc, char *argv[]) {
	cxxopts::Options options("metal-tools", "One line description of MyProgram");

	// clang-format off
	options.add_options()
			("s,stages", "Shader stage(s) to print. [vertex,fragment,compute]", cxxopts::value<std::vector<std::string>>())
			("filenames", "The filename(s) to process", cxxopts::value<std::vector<std::string>>())
			;
	// clang-format on

	options.parse_positional({ "filenames" });
	auto result = options.parse(argc, argv);

	std::set<RD::ShaderStage> stages;
	if (result.count("stages")) {
		auto stages_list = result["stages"].as<std::vector<std::string>>();
		for (auto &stage_str : stages_list) {
			auto stage = stage_from_string(stage_str);
			if (stage) {
				stages.insert(stage.value());
			}
		}
	}

	if (result.count("filenames") == 0) {
		std::cout << options.help() << std::endl;
		return 1;
	}

	auto filenames = result["filenames"].as<std::vector<std::string>>();

	for (auto &filename : filenames) {
		FILE *file = fopen(filename.c_str(), "rb");
		if (file == NULL) {
			perror("Error opening file");
			return 1;
		}

		fseek(file, 0, SEEK_END);
		long filesize = ftell(file);
		fseek(file, 0, SEEK_SET);

		std::vector<uint8_t> buffer;
		buffer.resize(filesize);

		fread(buffer.data(), 1, filesize, file);
		fclose(file);

		BufReader reader(buffer.data(), filesize);
		uint8_t header[4];
		reader.read((uint32_t &)header);
		if (memcmp(header, shader_file_header, 4) != 0) {
			printf("invalid header\n");
			exit(0);
		}
		uint32_t version = 0;
		reader.read(version);
		if (version != cache_file_version) {
			printf("invalid version\n");
			exit(0);
		}

		std::once_flag file_printed;

		uint32_t variant_count = 0;
		reader.read(variant_count);
		for (uint32_t i = 0; i < variant_count; i++) {
			uint32_t variant_size = 0;
			reader.read(variant_size);
			if (variant_size == 0) {
				continue;
			}

			reader.skip(8); // skip variant hash

			ShaderBinaryData binary_data;
			binary_data.deserialize(reader);
			switch (reader.status) {
				case BufReader::Status::OK:
					break;
				case BufReader::Status::BAD_COMPRESSION:
					exit(1);
				case BufReader::Status::SHORT_BUFFER:
					exit(1);
			}

			for (auto &shader_data : binary_data.stages) {
				if (!stages.empty() && stages.find(shader_data.stage) == stages.end()) {
					continue;
				}

				if (filenames.size() > 0) {
					std::call_once(file_printed, [filename]() {
						printf("file: %s\n", filename.c_str());
					});
				}

				auto stage = stage_to_string(shader_data.stage);
				printf("%s: type=%s\n", binary_data.shader_name.get(), stage);

				std::cout << shader_data.source.get() << std::endl;
			}
		}
	}

	return 0;
}

int zstd_level = 3;
bool zstd_long_distance_matching = false;
int zstd_window_log_size = 27; // ZSTD_WINDOWLOG_LIMIT_DEFAULT

int decompress(uint8_t *p_dst, int p_dst_max_size, const uint8_t *p_src, int p_src_size) {
	ZSTD_DCtx *dctx = ZSTD_createDCtx();
	if (zstd_long_distance_matching) {
		ZSTD_DCtx_setParameter(dctx, ZSTD_d_windowLogMax, zstd_window_log_size);
	}
	int ret = ZSTD_decompressDCtx(dctx, p_dst, p_dst_max_size, p_src, p_src_size);
	ZSTD_freeDCtx(dctx);
	return ret;
}
