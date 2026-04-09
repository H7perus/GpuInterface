#pragma once

//there is no reflection support here. The only reason this exists is so I can test VK_EXT_descriptor_heap


#include <vector>
#include <array>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <optional>
#include <set>
#include <fstream>
#include <filesystem>
#include "shaderc/shaderc.hpp"

std::vector<uint32_t> CompileGlslShader(const std::string& filepath, shaderc_shader_kind stage)
{
    // Read file
    std::ifstream file(filepath);
    if (!file.is_open())
        throw std::runtime_error("Failed to open shader file: " + filepath);

    std::stringstream ss;
    ss << file.rdbuf();
    std::string source = ss.str();

    // Compile
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
        source, stage, filepath.c_str(), options
    );

    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        std::cout << "Error compiling shader: " << result.GetErrorMessage() << std::endl;
        throw std::runtime_error("Shader compilation failed: " + result.GetErrorMessage());
    }
        

    return { result.cbegin(), result.cend() };
}