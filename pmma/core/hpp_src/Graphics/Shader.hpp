#pragma once
#include "Internal/Core/PMMA_Exports.hpp"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <future>
#include <string>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

namespace PMMA::Graphics {
class Shader {
private:
    std::future<void> CompileShaderFuture;

    std::string RawVertexShaderPath = "";
    std::string RawFragmentShaderPath = "";
    std::string CompiledVertexShaderPath = "";
    std::string CompiledFragmentShaderPath = "";

    bgfx::ProgramHandle ShaderProgram = BGFX_INVALID_HANDLE;

    bool IsCompiled = false;
    bool IsInternalShader = false;

    void CompileShader(bool InternalShader);

    void CompileShaderComponent(std::string RawFilePath, std::string CompiledFilePath, std::string Type);

    std::string GetGraphicsProfile();

public:
    ~Shader() {
        if (CompileShaderFuture.valid()) {
            CompileShaderFuture.wait();

            CompileShaderFuture = std::future<void>();
        }

        if (bgfx::isValid(ShaderProgram)) {
            bgfx::destroy(ShaderProgram);
        }
    }

    void CreateShader();

    void LoadShader(std::string VertexShaderPath, std::string FragmentShaderPath, bool InternalShader);

    void LoadVertexShader(std::string VertexShaderPath, bool InternalShader);

    void LoadFragmentShader(std::string FragmentShaderPath, bool InternalShader);

    void LoadShaderFromFolder(std::string FolderPath, bool InternalShader);

    bgfx::ProgramHandle Use();
};
} // namespace PMMA::Graphics