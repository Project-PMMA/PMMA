#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include "Internal/Core/PMMA_Core.hpp"
#include "Internal/Core/PMMA_Registry.hpp"

#include "Internal/LoggingManager.hpp"
#include "Internal/ParallelWorker.hpp"

#include "Graphics/Shader.hpp"

#include "Constants.hpp"
#include "General.hpp"
#include "Passport.hpp"

std::string PMMA::Graphics::Shader::GetGraphicsProfile() {
    std::string GraphicsBackend = PMMA::General::GetGraphicsBackend();
    if (GraphicsBackend == PMMA::Constants::GraphicsBackends::OPENGL_ES) {
        return "100_es";
    } else if (GraphicsBackend == PMMA::Constants::GraphicsBackends::DIRECT3D11 || GraphicsBackend == PMMA::Constants::GraphicsBackends::DIRECT3D12) {
        return "s_4_0";
    } else if (GraphicsBackend == PMMA::Constants::GraphicsBackends::METAL) {
        return "metal";
    } else if (GraphicsBackend == PMMA::Constants::GraphicsBackends::GNM) {
        return "pssl";
    } else if (GraphicsBackend == PMMA::Constants::GraphicsBackends::VULKAN) {
        return "spirv";
    } else if (GraphicsBackend == PMMA::Constants::GraphicsBackends::OPENGL) {
        return "150";
    } else {
        PMMA::Core::LoggingManagerInstance->InternalLogError(
            58,
            "Cannot compile shader as the graphics backend '" +
                GraphicsBackend + "' is not recognized. Please report \
this as a GitHub issue so we can add support for it.");

        throw std::runtime_error("Cannot compile shader for " + GraphicsBackend + " as its profile is not known.");
    }
}

const bgfx::Memory *InternalLoadShader(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    const bgfx::Memory *mem = bgfx::alloc(uint32_t(size));
    file.read((char *)mem->data, size);
    return mem;
}

void PMMA::Graphics::Shader::CompileShaderComponent(
    std::string RawFilePath,
    std::string CompiledFilePath,
    std::string Type) {

    bgfx::ShaderHandle shader_component = BGFX_INVALID_HANDLE;

    std::string PlatformName = PMMA::General::GetOperatingSystem();
    if (PlatformName == PMMA::Constants::OperatingSystems::ANDROID) {
        PlatformName = "android";
    } else if (PlatformName == PMMA::Constants::OperatingSystems::EMSCRIPTEN) {
        PlatformName = "asm.js";
    } else if (PlatformName == PMMA::Constants::OperatingSystems::IOS) {
        PlatformName = "ios";
    } else if (PlatformName == PMMA::Constants::OperatingSystems::LINUX) {
        PlatformName = "linux";
    } else if (PlatformName == PMMA::Constants::OperatingSystems::PS4) {
        PlatformName = "orbis";
    } else if (PlatformName == PMMA::Constants::OperatingSystems::MACOS) {
        PlatformName = "osx";
    } else if (PlatformName == PMMA::Constants::OperatingSystems::WINDOWS) {
        PlatformName = "windows";
    } else {
        throw std::runtime_error("Unsupported platform: " + PlatformName);
    }

    PMMA::Core::LoggingManagerInstance->InternalLogInfo(
        34,
        "PMMA is using '" + PlatformName + "' for shaders.");

    std::string Shader_C_Location = PMMA::Core::Registry::PMMA_Location +
                                    PMMA::Core::Registry::PathSeparator + "extern" +
                                    PMMA::Core::Registry::PathSeparator + "bin" +
                                    PMMA::Core::Registry::PathSeparator + "shaderc";

    if (PlatformName == "windows") {
        Shader_C_Location += ".exe";
    }

    std::string ShaderBuildToolsLocation = PMMA::Core::Registry::PMMA_Location +
                                           PMMA::Core::Registry::PathSeparator + "extern" +
                                           PMMA::Core::Registry::PathSeparator + "shader_build_tools";

    std::string VaryingDefLocation = std::filesystem::path(RawFilePath).parent_path().string() +
                                     PMMA::Core::Registry::PathSeparator + "varying.def.sc";

    std::string GraphicsProfile = PMMA::Graphics::Shader::GetGraphicsProfile();

    std::string command = "\"" + Shader_C_Location + "\"" +
                          " -f \"" + RawFilePath + "\"" +
                          " -o \"" + CompiledFilePath + "\"" +
                          " --type " + Type +
                          " --platform " + PlatformName +
                          " -i \"" + ShaderBuildToolsLocation + "\"" +
                          " --varyingdef \"" + VaryingDefLocation + "\"" +
                          " --profile " + GraphicsProfile;

#ifdef _WIN32
    std::string execution_command = "\"" + command + "\"";
#else
    std::string execution_command = command;
#endif

    if (!std::filesystem::exists(CompiledFilePath)) {
        std::filesystem::create_directories(
            std::filesystem::path(CompiledFilePath).parent_path());
    }

    bool DontRepeatOutput = false;

    try {
        if (system(execution_command.c_str()) != 0) {
            DontRepeatOutput = true;

            if (IsInternalShader) {
                PMMA::Core::LoggingManagerInstance->InternalLogError(
                    49,
                    "PMMA was unable to compile the following shader: '" +
                        RawFilePath + "'. This is a shader that comes included \
with PMMA, so please report this issue to us here: \
'https://github.com/Project-PMMA/PMMA/issues' so we can fix the issue. \
We would also greatly appreciate it if you could include all information/debug/warn/error \
logs from your application run to help us diagnose the issue as it could \
be specific to a single platform or graphics backend. Thank you!");
            } else {
                PMMA::Core::LoggingManagerInstance->InternalLogError(
                    49,
                    "Shader compilation failed for '" + RawFilePath +
                        "' with command: '" + execution_command + "'\n\n" +
                        "To diagnose this shader compilation issue, please run the \
command listed above in your system terminal/command prompt directly.");
            }
            throw std::runtime_error("Shader compilation failed for '" +
                                     RawFilePath + "' with command: '" + execution_command + "'.");
        }
    } catch (const std::exception &e) {
        if (!DontRepeatOutput) {
            PMMA::Core::LoggingManagerInstance->InternalLogError(
                49,
                "Shader compilation failed: '" + std::string(e.what()) + "'.");
            throw std::runtime_error("Shader compilation failed for '" +
                                     RawFilePath + "' with command: '" + execution_command +
                                     "'\nError: '" + std::string(e.what()) + "'.");
        }
        exit(49);
    }
}

void PMMA::Graphics::Shader::CreateShader() {
    if (CompileShaderFuture.valid()) {
        CompileShaderFuture.wait();

        CompileShaderFuture = std::future<void>();
    }

    if (IsCompiled) {
        return;
    }

    if (CompiledVertexShaderPath != "" && CompiledFragmentShaderPath != "") {
        bgfx::ShaderHandle vertex_shader = bgfx::createShader(
            InternalLoadShader(CompiledVertexShaderPath));
        bgfx::ShaderHandle fragment_shader = bgfx::createShader(
            InternalLoadShader(CompiledFragmentShaderPath));

        ShaderProgram = bgfx::createProgram(
            vertex_shader,
            fragment_shader,
            true);
        IsCompiled = true;
    } else {
        IsCompiled = false;
    }
}

void PMMA::Graphics::Shader::CompileShader(bool InternalShader) {
    IsInternalShader = InternalShader;

    std::string PlatformName = PMMA::General::GetOperatingSystem();

    if (RawVertexShaderPath != "") {
        if (CompiledVertexShaderPath == "") {
            std::string ShaderName = std::filesystem::path(RawVertexShaderPath).stem().string();
            if (InternalShader || !PMMA::Core::PassportInstance->IsRegistered) {
                CompiledVertexShaderPath = PMMA::Core::Registry::PMMA_Location + PMMA::Core::Registry::PathSeparator + "temporary" + PMMA::Core::Registry::PathSeparator + "shader_cache" + PMMA::Core::Registry::PathSeparator + PlatformName + PMMA::Core::Registry::PathSeparator + GetGraphicsProfile() + PMMA::Core::Registry::PathSeparator + ShaderName + ".bin";
            } else {
                CompiledVertexShaderPath = PMMA::Core::PassportInstance->GetTemporaryPath() + PMMA::Core::Registry::PathSeparator + "shader_cache" + PMMA::Core::Registry::PathSeparator + PlatformName + PMMA::Core::Registry::PathSeparator + GetGraphicsProfile() + PMMA::Core::Registry::PathSeparator + ShaderName + ".bin";
            }
        }
    }

    if (RawFragmentShaderPath != "") {
        if (CompiledFragmentShaderPath == "") {
            std::string ShaderName = std::filesystem::path(RawFragmentShaderPath).stem().string();
            if (InternalShader || !PMMA::Core::PassportInstance->GetIsRegistered()) {
                CompiledFragmentShaderPath = PMMA::Core::Registry::PMMA_Location + PMMA::Core::Registry::PathSeparator + "temporary" + PMMA::Core::Registry::PathSeparator + "shader_cache" + PMMA::Core::Registry::PathSeparator + PlatformName + PMMA::Core::Registry::PathSeparator + GetGraphicsProfile() + PMMA::Core::Registry::PathSeparator + ShaderName + ".bin";
            } else {
                CompiledFragmentShaderPath = PMMA::Core::PassportInstance->GetTemporaryPath() + PMMA::Core::Registry::PathSeparator + "shader_cache" + PMMA::Core::Registry::PathSeparator + PlatformName + PMMA::Core::Registry::PathSeparator + GetGraphicsProfile() + PMMA::Core::Registry::PathSeparator + ShaderName + ".bin";
            }
        }
    }

    if (!std::filesystem::exists(CompiledVertexShaderPath)) {
        if (RawVertexShaderPath != "") {
            CompileShaderComponent(RawVertexShaderPath, CompiledVertexShaderPath, "vertex");
        }
    }
    if (!std::filesystem::exists(CompiledFragmentShaderPath)) {
        if (RawFragmentShaderPath != "") {
            CompileShaderComponent(RawFragmentShaderPath, CompiledFragmentShaderPath, "fragment");
        }
    }
}

bgfx::ProgramHandle PMMA::Graphics::Shader::Use() {
    if (CompileShaderFuture.valid()) {
        CompileShaderFuture.wait();

        CompileShaderFuture = std::future<void>();
    }

    if (IsCompiled) {
        return ShaderProgram;
    } else {
        throw std::runtime_error("Shader is not compiled");
    }

    if (RawVertexShaderPath == "" || CompiledVertexShaderPath == "") {
        throw std::runtime_error("Vertex shader path is not set");
    }

    if (RawFragmentShaderPath == "" || CompiledFragmentShaderPath == "") {
        throw std::runtime_error("Fragment shader path is not set");
    }
}

void PMMA::Graphics::Shader::LoadShader(std::string VertexShaderPath, std::string FragmentShaderPath, bool InternalShader) {
    if (CompileShaderFuture.valid()) {
        CompileShaderFuture.wait();

        CompileShaderFuture = std::future<void>();
    }

    if (VertexShaderPath.size() >= 5 && VertexShaderPath.substr(VertexShaderPath.size() - 5) == ".bin") {
        IsCompiled = true;
        CompiledVertexShaderPath = VertexShaderPath;
        RawVertexShaderPath = "";
    } else {
        IsCompiled = false;
        RawVertexShaderPath = VertexShaderPath;
        CompiledVertexShaderPath = "";
    }

    if (FragmentShaderPath.size() >= 5 && FragmentShaderPath.substr(FragmentShaderPath.size() - 5) == ".bin") {
        IsCompiled = true;
        CompiledFragmentShaderPath = FragmentShaderPath;
        RawFragmentShaderPath = "";
    } else {
        IsCompiled = false;
        RawFragmentShaderPath = FragmentShaderPath;
        CompiledFragmentShaderPath = "";
    }

    PMMA::Core::ParallelWorkerInstance->ShadersToLoad++;
    CompileShaderFuture = PMMA::Core::ParallelWorkerInstance->Enqueue([this, InternalShader]() {
        CompileShader(InternalShader);
        PMMA::Core::ParallelWorkerInstance->ShadersLoaded++;
    });
}

void PMMA::Graphics::Shader::LoadVertexShader(std::string VertexShaderPath, bool InternalShader) {
    if (CompileShaderFuture.valid()) {
        CompileShaderFuture.wait();

        CompileShaderFuture = std::future<void>();
    }

    if (VertexShaderPath.size() >= 5 && VertexShaderPath.substr(VertexShaderPath.size() - 5) == ".bin") {
        IsCompiled = true;
        CompiledVertexShaderPath = VertexShaderPath;
        RawVertexShaderPath = "";
    } else {
        IsCompiled = false;
        RawVertexShaderPath = VertexShaderPath;
        CompiledVertexShaderPath = "";
    }

    PMMA::Core::ParallelWorkerInstance->ShadersToLoad++;
    CompileShaderFuture = PMMA::Core::ParallelWorkerInstance->Enqueue([this, InternalShader]() {
        CompileShader(InternalShader);
        PMMA::Core::ParallelWorkerInstance->ShadersLoaded++;
    });
}

void PMMA::Graphics::Shader::LoadFragmentShader(std::string FragmentShaderPath, bool InternalShader) {
    if (CompileShaderFuture.valid()) {
        CompileShaderFuture.wait();

        CompileShaderFuture = std::future<void>();
    }

    if (FragmentShaderPath.size() >= 5 && FragmentShaderPath.substr(FragmentShaderPath.size() - 5) == ".bin") {
        IsCompiled = true;
        CompiledFragmentShaderPath = FragmentShaderPath;
        RawFragmentShaderPath = "";
    } else {
        IsCompiled = false;
        RawFragmentShaderPath = FragmentShaderPath;
        CompiledFragmentShaderPath = "";
    }

    PMMA::Core::ParallelWorkerInstance->ShadersToLoad++;
    CompileShaderFuture = PMMA::Core::ParallelWorkerInstance->Enqueue([this, InternalShader]() {
        CompileShader(InternalShader);
        PMMA::Core::ParallelWorkerInstance->ShadersLoaded++;
    });
}

void PMMA::Graphics::Shader::LoadShaderFromFolder(std::string FolderPath, bool InternalShader) {
    if (CompileShaderFuture.valid()) {
        CompileShaderFuture.wait();

        CompileShaderFuture = std::future<void>();
    }

    try {
        for (const auto &entry : std::filesystem::directory_iterator(FolderPath)) {
            std::string FileName = entry.path().filename().string();
            std::string FilePath = entry.path().string();

            if (FileName.size() >= 7 && FileName.substr(FileName.size() - 5) == ".bin") {
                IsCompiled = true;
            } else {
                IsCompiled = false;
            }

            std::string LowerFileName = FileName;
            std::transform(LowerFileName.begin(), LowerFileName.end(), LowerFileName.begin(), ::tolower);
            if (LowerFileName.find("vertex") != std::string::npos) {
                if (IsCompiled) {
                    CompiledVertexShaderPath = FilePath;
                    RawVertexShaderPath = "";
                } else {
                    RawVertexShaderPath = FilePath;
                    CompiledVertexShaderPath = "";
                }
            } else if (LowerFileName.find("fragment") != std::string::npos) {
                if (IsCompiled) {
                    CompiledFragmentShaderPath = FilePath;
                    RawFragmentShaderPath = "";
                } else {
                    RawFragmentShaderPath = FilePath;
                    CompiledFragmentShaderPath = "";
                }
            } else if (LowerFileName.substr(0, 3) == "vs_") {
                if (IsCompiled) {
                    CompiledVertexShaderPath = FilePath;
                    RawVertexShaderPath = "";
                } else {
                    RawVertexShaderPath = FilePath;
                    CompiledVertexShaderPath = "";
                }
            } else if (LowerFileName.substr(0, 3) == "fs_") {
                if (IsCompiled) {
                    CompiledFragmentShaderPath = FilePath;
                    RawFragmentShaderPath = "";
                } else {
                    RawFragmentShaderPath = FilePath;
                    CompiledFragmentShaderPath = "";
                }
            }

            if ((CompiledVertexShaderPath != "" || RawVertexShaderPath != "") &&
                (CompiledFragmentShaderPath != "" || RawFragmentShaderPath != "")) {
                break;
            }
        }
    } catch (const std::filesystem::filesystem_error &error) {
        PMMA::Core::LoggingManagerInstance->InternalLogWarn(
            48,
            "Whilst looking for shader files in the folder: '" +
                FolderPath + "' the following filesystem error occurred: '" +
                error.what() + "'");
    }

    PMMA::Core::ParallelWorkerInstance->ShadersToLoad++;
    CompileShaderFuture = PMMA::Core::ParallelWorkerInstance->Enqueue([this, InternalShader]() {
        CompileShader(InternalShader);
        PMMA::Core::ParallelWorkerInstance->ShadersLoaded++;
    });
}