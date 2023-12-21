#pragma once

#include <iostream>
#include "webgpu/webgpu.h"
#include <fstream>
#include <string>

namespace med {

	// Making just a util class for now
	struct Shader
	{
		static std::string load_shader_string(const char* path)
		{
			std::ifstream stream(path);
			std::string source;

			if (!stream.is_open())
			{
				std::cout << "Couldn't open file: " << path << "\n";
				return source;
			}

			std::string line;
			while (std::getline(stream, line))
			{
				source += line + '\n';
			}

			return source;
		}

		static WGPUShaderModule create_shader_module(WGPUDevice device, std::string source)
		{
			WGPUShaderModuleWGSLDescriptor w_desc;
			w_desc.chain.next = nullptr;
			w_desc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
			w_desc.code = source.c_str();

			WGPUShaderModuleDescriptor desc{};
			desc.nextInChain = &w_desc.chain;

			return wgpuDeviceCreateShaderModule(device, &desc);
		}

		static WGPUShaderModule create_shader_module_from_path(WGPUDevice device, const char* path)
		{
			return Shader::create_shader_module(device, Shader::load_shader_string(path));
		}
	};

}
