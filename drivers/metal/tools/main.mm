//
// Created by Stuart Carnie on 26/10/2023.
//

#include "thirdparty/cxxopts/include/cxxopts.hpp"
#include "types.h"
#include "zstd.h"

#include <stdint.h>
#include <fstream>
#include <iostream>
#include <vector>

constexpr auto const MODE = cista::mode::NONE;

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
//			("d,debug", "Enable debugging") // a bool parameter
//			("i,integer", "Int param", cxxopts::value<int>())
//			("f,file", "File name", cxxopts::value<std::string>())
//			("v,verbose", "Verbose output", cxxopts::value<bool>()->default_value("false"))
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

		std::vector<char> buffer;
		buffer.resize(filesize);

		fread(buffer.data(), 1, filesize, file);
		fclose(file);

		Reader f(buffer);

		char header[5] = { 0, 0, 0, 0, 0 };
		f.get_buffer((uint8_t *)header, 4);
		if (strcmp(header, shader_file_header) != 0) {
			return -1;
		}

		uint32_t file_version = f.get_32();
		if (file_version != cache_file_version) {
			return 1; // wrong version
		}

		std::once_flag file_printed;

		uint32_t variant_count = f.get_32();
		for (uint32_t i = 0; i < variant_count; i++) {
			uint32_t variant_size = f.get_32();
			if (variant_size == 0) {
				continue;
			}
			std::vector<uint8_t> buffer;
			buffer.resize(variant_size);

			f.get_buffer(buffer.data(), variant_size);

			const uint8_t *binptr = buffer.data() + 8;
			uint32_t binsize = buffer.size() - 8;

			try {
				auto const binary_data = cista::deserialize<ShaderBinaryData, MODE>(binptr, binptr + binsize);
				std::string name = binary_data->shader_name.str();

				for (auto &shader_data : binary_data->stages) {
					if (!stages.empty() && stages.find(shader_data.stage) == stages.end()) {
						continue;
					}

					if (filenames.size() > 0) {
						std::call_once(file_printed, [filename]() {
							printf("file: %s\n", filename.c_str());
						});
					}

					auto stage = stage_to_string(shader_data.stage);
					printf("%s: type=%s\n", name.c_str(), stage);

					size_t bufsize = shader_data.source_size;
					uint8_t *buf = static_cast<uint8_t *>(malloc(bufsize + 1));
					int decoded_size = decompress(buf, bufsize, shader_data.source_data.data(), shader_data.source_data.size());
					if (decoded_size != bufsize) {
						// do something
					};
					buf[bufsize] = 0;

					std::cout << (char *)buf << std::endl;
					free(buf);
				}
			} catch (cista::cista_exception &e) {
				std::cerr << e.what() << std::endl;
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
