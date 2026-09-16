#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <string>
#include <vector>

#include "Internal/Core/PMMA_Core.hpp"
#include "Internal/Core/PMMA_Registry.hpp"

#include "Internal/Rendering/Core2D/RenderPipelineInstance.hpp"
#include "Internal/Rendering/Core2D/RenderPipelineManager.hpp"

#include "Internal/LoggingManager.hpp"
#include "Internal/ParallelWorker.hpp"
#include "Internal/SplashScreen.hpp"

#include "Rendering/Shapes2D/ArcShape.hpp"
#include "Rendering/Shapes2D/LineShape.hpp"
#include "Rendering/Shapes2D/PixelShape.hpp"
#include "Rendering/Shapes2D/RadialPolygonShape.hpp"
#include "Rendering/Shapes2D/RectangleShape.hpp"

#include "Display.hpp"
#include "Graphics/Shader.hpp"

PMMA::Internal::Rendering::Core2D::RenderPipelineInstance::RenderPipelineInstance() {
    ID = reinterpret_cast<uintptr_t>(this);

    const bgfx::Caps *caps = bgfx::getCaps();
    MaxTextureDimension = std::min(
        static_cast<uint32_t>(caps->limits.maxTextureSize),
        static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()));

    CompressedTextureManager.Initialize(ID, MaxTextureDimension);

    m_layout.begin()
        .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    instanceLayout.begin()
        .add(bgfx::Attrib::TexCoord7, 4, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord6, 4, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord5, 4, bgfx::AttribType::Float)
        .end();

    s_colorTex = bgfx::createUniform(
        "s_colorTex",
        bgfx::UniformType::Sampler);

    u_textureInfo = bgfx::createUniform(
        "u_textureInfo",
        bgfx::UniformType::Vec4);

    OrthDisplayProj = bgfx::createUniform(
        "OrthDisplayProj",
        bgfx::UniformType::Mat4);

    u_FragmentData = bgfx::createUniform(
        "FragmentData",
        bgfx::UniformType::Vec4);

    VertexData[0] = {-0.5f, -0.5f, 0.0f, 0.0f};
    VertexData[1] = {0.5f, -0.5f, 1.0f, 0.0f};
    VertexData[2] = {0.5f, 0.5f, 1.0f, 1.0f};
    VertexData[3] = {-0.5f, 0.5f, 0.0f, 1.0f};

    IndexData[0] = 0;
    IndexData[1] = 1;
    IndexData[2] = 2;
    IndexData[3] = 0;
    IndexData[4] = 2;
    IndexData[5] = 3;

    vbh = bgfx::createVertexBuffer(
        bgfx::makeRef(
            VertexData,
            sizeof(Vertex) * 4),
        m_layout);

    ibh = bgfx::createIndexBuffer(
        bgfx::makeRef(
            IndexData,
            sizeof(uint16_t) * 6));

    OpaqueInstanceVbh = bgfx::createDynamicVertexBuffer(
        OpaqueInstanceCount,
        instanceLayout);

    TransparentInstanceVbh = bgfx::createDynamicVertexBuffer(
        TransparentInstanceCount,
        instanceLayout);

    ColorTexture.MaxTextureDimension = MaxTextureDimension;
}

void PMMA::Internal::Rendering::Core2D::RenderPipelineInstance::AdvanceView() {
    PMMA::Core::Registry::RollingViewID++;
    if (PMMA::Core::Registry::RollingViewID >= PMMA::Core::Registry::MaxViewID) {
        PMMA::Core::LoggingManagerInstance->InternalLogError(
            64,
            "The maximum number of internal views has been exceeded. \
Please consider reducing the number of windows or quantity of 2D shapes \
rendered if rendering more than 16,777,216 shapes to a single window.");

        throw std::runtime_error("The maximum number of internal views has been exceeded.");
    }

    bgfx::setViewRect(
        PMMA::Core::Registry::RollingViewID,
        0,
        0,
        PMMA::Core::ActiveDisplayInstance->GetWidth(),
        PMMA::Core::ActiveDisplayInstance->GetHeight());

    bgfx::setViewFrameBuffer(
        PMMA::Core::Registry::RollingViewID,
        PMMA::Core::ActiveDisplayInstance->DisplayFrameBufferHandle);

    bgfx::setViewClear(
        PMMA::Core::Registry::RollingViewID,
        BGFX_CLEAR_DEPTH,
        0,
        1.0f,
        0);

    bgfx::touch(PMMA::Core::Registry::RollingViewID);
}

template <typename T>
void PMMA::Internal::Rendering::Core2D::RenderPipelineInstance::Add(T *shape, uint16_t *TextureSize, unsigned char Channels) {
    uintptr_t ShapeID = shape->ID;
    const bool ColorDataChanged = shape->ColorDataChanged;
    const bool PropertyChanged = shape->ShapePropertyChanged;
    auto &Color = shape->Color;
    auto &Texture = shape->Texture;
    auto &instance = shape->ShapeInstanceData;

    bool IsOpaque = Color.IsOpaque();

    instance.color_index = ColorTexture.AddColor(&Color, ShapeID, ColorDataChanged);

    // Used for RGBA generated texture for noise and text
    instance.texture_position = 0;
    instance.texture_size = 0;
    // end

    if (PMMA::Core::MasterDisplayInstance->RenderPipelineCore->FirstFrame) {
        PMMA::Core::MasterDisplayInstance->RenderPipelineCore->FirstFrame = false;

        if (PMMA::Core::ParallelWorkerInstance->ShadersToLoad + PMMA::Core::ParallelWorkerInstance->TexturesToLoad + PMMA::Core::ParallelWorkerInstance->FontsToLoad > 1) {
            PMMA::Internal::SplashScreen SplashScreen;

            SplashScreen.Play();

            PMMA::Core::Registry::TextureCompilationStartTime.reset();
            PMMA::Core::Registry::InitialSetup = false;
        }
    }

    // CompressedTexture
    uint16_t TexturePositionInAtlas[2] = {0, 0};
    if (Texture.IsEnabled()) {
        if (TextureSize[0] > MaxTextureDimension || TextureSize[1] > MaxTextureDimension) {
            std::cout << "The texture: " << Texture.GetPath() << " is too large with dimensions: " << TextureSize[0] << "x" << TextureSize[1] << std::endl;

            return;
        }

        float TextureID = 0;

        if (Channels == 3) {
            TextureID = CompressedTextureManager.RegisterOpaque(Texture.TextureProperties);
        } else {
            TextureID = CompressedTextureManager.RegisterTransparent(Texture.TextureProperties);
            IsOpaque = false;
        }

        instance.texture_id = TextureID;
    }

    ColorChanged |= ColorDataChanged;
    ShapePropertyChanged |= PropertyChanged;

    if (IsOpaque) { // opaque
        if (UsingCache && OpaqueInstanceCount < CurrentOpaqueDataSize && CurrentShapeIDs[0][OpaqueInstanceCount] == ShapeID) {
            if (PropertyChanged) {
                CurrentInstanceData[0][OpaqueInstanceCount] = instance;
            }

            OpaqueInstanceCount++;
            return;
        }

        UsingCache = false;

        if (OpaqueInstanceCount >= CurrentOpaqueDataSize) {
            CurrentInstanceData[0].push_back(instance);
            CurrentShapeIDs[0].push_back(ShapeID);
            CurrentOpaqueDataSize++;
        } else {
            CurrentInstanceData[0][OpaqueInstanceCount] = instance;
            CurrentShapeIDs[0][OpaqueInstanceCount] = ShapeID;
        }

        OpaqueInstanceCount++;
    } else { // transparent
        if (UsingCache && TransparentInstanceCount < CurrentTransparentDataSize && CurrentShapeIDs[1][TransparentInstanceCount] == ShapeID) {
            if (PropertyChanged) {
                CurrentInstanceData[1][TransparentInstanceCount] = instance;
            }

            TransparentInstanceCount++;
            return;
        }

        UsingCache = false;

        if (TransparentInstanceCount >= CurrentTransparentDataSize) {
            CurrentInstanceData[1].push_back(instance);
            CurrentShapeIDs[1].push_back(ShapeID);
            CurrentTransparentDataSize++;
        } else {
            CurrentInstanceData[1][TransparentInstanceCount] = instance;
            CurrentShapeIDs[1][TransparentInstanceCount] = ShapeID;
        }

        TransparentInstanceCount++;
    }
}

template void
PMMA::Internal::Rendering::Core2D::RenderPipelineInstance::Add<PMMA::Rendering::TwoD::Shapes::Rectangle>(
    PMMA::Rendering::TwoD::Shapes::Rectangle *,
    uint16_t *,
    unsigned char);

template void
PMMA::Internal::Rendering::Core2D::RenderPipelineInstance::Add<PMMA::Rendering::TwoD::Shapes::Arc>(
    PMMA::Rendering::TwoD::Shapes::Arc *,
    uint16_t *,
    unsigned char);

template void
PMMA::Internal::Rendering::Core2D::RenderPipelineInstance::Add<PMMA::Rendering::TwoD::Shapes::Line>(
    PMMA::Rendering::TwoD::Shapes::Line *,
    uint16_t *,
    unsigned char);

template void
PMMA::Internal::Rendering::Core2D::RenderPipelineInstance::Add<PMMA::Rendering::TwoD::Shapes::RadialPolygonBase>(
    PMMA::Rendering::TwoD::Shapes::RadialPolygonBase *,
    uint16_t *,
    unsigned char);

template <typename T>
void PMMA::Internal::Rendering::Core2D::RenderPipelineInstance::Add(T *shape) {
    uintptr_t ShapeID = shape->ID;
    const bool ColorDataChanged = shape->ColorDataChanged;
    const bool PropertyChanged = shape->ShapePropertyChanged;
    auto &Color = shape->Color;
    auto &instance = shape->ShapeInstanceData;

    bool IsOpaque = Color.IsOpaque();

    instance.color_index = ColorTexture.AddColor(&Color, ShapeID, ColorDataChanged);

    if (PMMA::Core::MasterDisplayInstance->RenderPipelineCore->FirstFrame) {
        PMMA::Core::MasterDisplayInstance->RenderPipelineCore->FirstFrame = false;

        if (PMMA::Core::ParallelWorkerInstance->ShadersToLoad + PMMA::Core::ParallelWorkerInstance->TexturesToLoad + PMMA::Core::ParallelWorkerInstance->FontsToLoad > 1) {
            PMMA::Internal::SplashScreen SplashScreen;
            SplashScreen.Play();

            PMMA::Core::Registry::TextureCompilationStartTime.reset();
            PMMA::Core::Registry::InitialSetup = false;
        }
    }

    ColorChanged |= ColorDataChanged;
    ShapePropertyChanged |= PropertyChanged;

    if (IsOpaque) { // opaque
        if (UsingCache && OpaqueInstanceCount < CurrentOpaqueDataSize && CurrentShapeIDs[0][OpaqueInstanceCount] == ShapeID) {
            if (PropertyChanged) {
                CurrentInstanceData[0][OpaqueInstanceCount] = instance;
            }

            OpaqueInstanceCount++;
            return;
        }

        UsingCache = false;

        if (OpaqueInstanceCount >= CurrentOpaqueDataSize) {
            CurrentInstanceData[0].push_back(instance);
            CurrentShapeIDs[0].push_back(ShapeID);
            CurrentOpaqueDataSize++;
        } else {
            CurrentInstanceData[0][OpaqueInstanceCount] = instance;
            CurrentShapeIDs[0][OpaqueInstanceCount] = ShapeID;
        }

        OpaqueInstanceCount++;
    } else { // transparent
        if (UsingCache && TransparentInstanceCount < CurrentTransparentDataSize && CurrentShapeIDs[1][TransparentInstanceCount] == ShapeID) {
            if (PropertyChanged) {
                CurrentInstanceData[1][TransparentInstanceCount] = instance;
            }

            TransparentInstanceCount++;
            return;
        }

        UsingCache = false;

        if (TransparentInstanceCount >= CurrentTransparentDataSize) {
            CurrentInstanceData[1].push_back(instance);
            CurrentShapeIDs[1].push_back(ShapeID);
            CurrentTransparentDataSize++;
        } else {
            CurrentInstanceData[1][TransparentInstanceCount] = instance;
            CurrentShapeIDs[1][TransparentInstanceCount] = ShapeID;
        }

        TransparentInstanceCount++;
    }
}

template void
PMMA::Internal::Rendering::Core2D::RenderPipelineInstance::Add<PMMA::Rendering::TwoD::Shapes::Pixel>(
    PMMA::Rendering::TwoD::Shapes::Pixel *);

void PMMA::Internal::Rendering::Core2D::RenderPipelineInstance::Render() {
    if (ColorChanged || !ColorTexture.UsingCache) {
        PMMA::Core::ActiveDisplayInstance->TriggerEventRefresh();

        ColorTexture.Assemble();
    }

    // GeneratedTextureManager.Assemble();
    CompressedTextureManager.Assemble();

    if (ShapePropertyChanged || OpaqueInstanceCount != OpaquePreviousBufferSize || TransparentInstanceCount != TransparentPreviousBufferSize || !bgfx::isValid(OpaqueInstanceVbh) || !bgfx::isValid(TransparentInstanceVbh)) {
        PMMA::Core::ActiveDisplayInstance->TriggerEventRefresh();

        CurrentInstanceData[0].resize(OpaqueInstanceCount); // Free memory if instanceCount decreased
        CurrentInstanceData[0].shrink_to_fit();             // Free memory if instanceCount decreased

        CurrentInstanceData[1].resize(TransparentInstanceCount); // Free memory if instanceCount decreased
        CurrentInstanceData[1].shrink_to_fit();                  // Free memory if instanceCount decreased

        CurrentShapeIDs[0].resize(OpaqueInstanceCount);
        CurrentShapeIDs[0].shrink_to_fit();

        CurrentShapeIDs[1].resize(TransparentInstanceCount);
        CurrentShapeIDs[1].shrink_to_fit();

        PreviousShapeIDs[BufferID] = CurrentShapeIDs;

        {
            // opaque first
            const uint32_t ArrayLength =
                static_cast<uint32_t>(CurrentInstanceData[0].size());

            // Keep PreviousInstanceData in logical/Add order.
            PreviousInstanceData[BufferID][0] = CurrentInstanceData[0];

            // Build a separate reversed buffer for GPU rendering.
            //
            // Logical order:
            //     A B C D
            //
            // GPU/depth order:
            //     D C B A
            //
            OpaqueGPUInstanceData[BufferID].resize(ArrayLength);

            if (ArrayLength > 0) {
                const auto *src =
                    CurrentInstanceData[0].data() + ArrayLength - 1;

                auto *dest =
                    OpaqueGPUInstanceData[BufferID].data();

                const auto *end =
                    dest + ArrayLength;

                while (dest < end) {
                    *dest++ = *src--;
                }

                const bgfx::Memory *OpaqueInstanceDataMem =
                    bgfx::makeRef(
                        OpaqueGPUInstanceData[BufferID].data(),
                        ArrayLength * sizeof(InstanceData));

                if (bgfx::isValid(OpaqueInstanceVbh)) {
                    if (ArrayLength != OpaquePreviousBufferSize) {
                        bgfx::destroy(OpaqueInstanceVbh);

                        OpaqueInstanceVbh =
                            bgfx::createDynamicVertexBuffer(
                                OpaqueInstanceDataMem,
                                instanceLayout);
                    } else {
                        bgfx::update(
                            OpaqueInstanceVbh,
                            0,
                            OpaqueInstanceDataMem);
                    }
                } else {
                    OpaqueInstanceVbh =
                        bgfx::createDynamicVertexBuffer(
                            OpaqueInstanceDataMem,
                            instanceLayout);
                }
            }

            OpaquePreviousBufferSize = ArrayLength;
        }
        {
            // then transparent

            // Ensure destination has the exact required memory allocated
            PreviousInstanceData[BufferID][1] = CurrentInstanceData[1];

            uint32_t CurrentBufferSize = PreviousInstanceData[BufferID][1].size();

            if (CurrentBufferSize > 0) {
                const bgfx::Memory *TransparentInstanceDataMem = bgfx::makeRef(
                    PreviousInstanceData[BufferID][1].data(),
                    CurrentBufferSize * sizeof(InstanceData));

                if (bgfx::isValid(TransparentInstanceVbh)) {
                    if (CurrentBufferSize != TransparentPreviousBufferSize) {
                        bgfx::destroy(TransparentInstanceVbh);
                        TransparentInstanceVbh = bgfx::createDynamicVertexBuffer(
                            TransparentInstanceDataMem,
                            instanceLayout);
                    } else {
                        bgfx::update(
                            TransparentInstanceVbh,
                            0,
                            TransparentInstanceDataMem);
                    }
                } else {
                    TransparentInstanceVbh = bgfx::createDynamicVertexBuffer(
                        TransparentInstanceDataMem,
                        instanceLayout);
                }
            }

            TransparentPreviousBufferSize = CurrentBufferSize;
        }
        // end

        PreviousBufferID = BufferID;
        BufferID = (BufferID + 1) % 4;
    }

    // opaque
    if (OpaquePreviousBufferSize > 0) {
        float proj[16];
        PMMA::Core::ActiveDisplayInstance->GetOrthographicProjection(proj);
        bgfx::setUniform(OrthDisplayProj, proj);

        float textureInfo[4] = {
            float(ColorTexture.m_colorTextureWidth),
            float(ColorTexture.m_colorTextureHeight),
            0,
            0};

        GeneratedTextureManager.OpaquePass();
        CompressedTextureManager.OpaquePass(textureInfo);

        bgfx::setUniform(u_textureInfo, textureInfo);

        bgfx::setVertexBuffer(0, vbh);
        bgfx::setIndexBuffer(ibh);

        bgfx::setTexture(0, s_colorTex, ColorTexture.ColorTextureHandle, BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_POINT);
        // bgfx::setTexture(1, RGB texture for generated textures like text and noise)

        float FragmentData[4] = {1.0f, 0.0f, 0.0f, 0.0f};
        CompressedTextureManager.WriteOpaqueData(FragmentData);

        bgfx::setUniform(u_FragmentData, FragmentData);

        bgfx::setInstanceDataBuffer(
            OpaqueInstanceVbh,
            0,
            OpaquePreviousBufferSize);

        bgfx::setState(
            BGFX_STATE_WRITE_RGB |
            BGFX_STATE_WRITE_A |
            BGFX_STATE_DEPTH_TEST_LEQUAL |
            BGFX_STATE_WRITE_Z);

        bgfx::submit(
            PMMA::Core::Registry::RollingViewID,
            PMMA::Core::Core2D_ShapeSDF_Program->Use());
    }

    // transparent
    if (TransparentPreviousBufferSize > 0) {
        float proj[16];
        PMMA::Core::ActiveDisplayInstance->GetOrthographicProjection(proj);
        bgfx::setUniform(OrthDisplayProj, proj);

        float CompressedTextureWidth = 0;
        float CompressedTextureHeight = 0;

        float textureInfo[4] = {
            float(ColorTexture.m_colorTextureWidth),
            float(ColorTexture.m_colorTextureHeight),
            0,
            0};

        GeneratedTextureManager.TransparentPass();
        CompressedTextureManager.TransparentPass(textureInfo);

        bgfx::setUniform(u_textureInfo, textureInfo);

        bgfx::setVertexBuffer(0, vbh);
        bgfx::setIndexBuffer(ibh);

        bgfx::setTexture(0, s_colorTex, ColorTexture.ColorTextureHandle, BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_POINT);
        // bgfx::setTexture(1, RGBA texture for generated textures like text and noise)

        float FragmentData[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        CompressedTextureManager.WriteTransparentData(FragmentData);
        bgfx::setUniform(u_FragmentData, FragmentData);

        bgfx::setInstanceDataBuffer(
            TransparentInstanceVbh,
            0,
            TransparentPreviousBufferSize);

        bgfx::setState(
            BGFX_STATE_WRITE_RGB |
            BGFX_STATE_WRITE_A |
            BGFX_STATE_BLEND_ALPHA |
            BGFX_STATE_DEPTH_TEST_LEQUAL);

        bgfx::submit(
            PMMA::Core::Registry::RollingViewID,
            PMMA::Core::Core2D_ShapeSDF_Program->Use());
    }

    AdvanceView();
}