#pragma once
#include "Internal/Core/PMMA_Exports.hpp"

#include <cstdint>
#include <vector>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include "Internal/Rendering/Core2D/ColorTextureManager.hpp"
#include "Internal/Rendering/Core2D/CompressedTextureManager.hpp"
#include "Internal/Rendering/Core2D/GeneratedTextureManager.hpp"
#include "Internal/Rendering/Core2D/RenderPipelineManager.hpp"

namespace PMMA::Internal::Rendering::Core2D {
class EXPORT RenderPipelineInstance {
private:
    std::array<std::array<std::vector<uintptr_t>, 2>, 4> PreviousShapeIDs;
    std::array<std::vector<uintptr_t>, 2> CurrentShapeIDs;

    std::array<std::array<std::vector<InstanceData>, 2>, 4> PreviousInstanceData;
    std::array<std::vector<InstanceData>, 2> CurrentInstanceData; // opaque, transparent
    std::array<std::vector<InstanceData>, 4> OpaqueGPUInstanceData;

    PMMA::Internal::Rendering::Core2D::ColorTextureManager ColorTexture;

public:
    PMMA::Internal::Rendering::Core2D::GeneratedTextureManager GeneratedTextureManager;
    PMMA::Internal::Rendering::Core2D::CompressedTextureManager CompressedTextureManager;

private:
    Vertex VertexData[4];
    uint16_t IndexData[6];

    bgfx::VertexLayout m_layout;
    bgfx::VertexLayout instanceLayout;

    uint32_t numTiles;

public:
    uintptr_t ID;

    uint32_t OpaqueInstanceCount = 0;
    uint32_t TransparentInstanceCount = 0;
    uint32_t MaxTextureDimension;

private:
    uint32_t OpaquePreviousBufferSize = 0;
    uint32_t TransparentPreviousBufferSize = 0;
    uint32_t CurrentOpaqueDataSize = 0;
    uint32_t CurrentTransparentDataSize = 0;

    bgfx::VertexBufferHandle vbh;
    bgfx::IndexBufferHandle ibh;
    bgfx::UniformHandle OrthDisplayProj;

    bgfx::DynamicVertexBufferHandle OpaqueInstanceVbh = BGFX_INVALID_HANDLE;
    bgfx::DynamicVertexBufferHandle TransparentInstanceVbh = BGFX_INVALID_HANDLE;

    bgfx::UniformHandle u_textureInfo;
    bgfx::UniformHandle u_FragmentData;
    bgfx::UniformHandle s_colorTex;

    char BufferID = 0;
    char PreviousBufferID = 0;

    bool ColorChanged = true;
    bool ShapePropertyChanged = true;
    bool UsingCache = false;

public:
    RenderPipelineInstance();

    ~RenderPipelineInstance() {
        if (bgfx::isValid(vbh)) {
            bgfx::destroy(vbh);
        }
        if (bgfx::isValid(ibh)) {
            bgfx::destroy(ibh);
        }
        if (bgfx::isValid(s_colorTex)) {
            bgfx::destroy(s_colorTex);
        }
        if (bgfx::isValid(u_textureInfo)) {
            bgfx::destroy(u_textureInfo);
        }
        if (bgfx::isValid(OrthDisplayProj)) {
            bgfx::destroy(OrthDisplayProj);
        }

        if (bgfx::isValid(OpaqueInstanceVbh)) {
            bgfx::destroy(OpaqueInstanceVbh);
        }
        if (bgfx::isValid(TransparentInstanceVbh)) {
            bgfx::destroy(TransparentInstanceVbh);
        }

        if (bgfx::isValid(u_FragmentData)) {
            bgfx::destroy(u_FragmentData);
        }
    }

    void AdvanceView();

    inline void Reset() {
        ColorTexture.Reset();
        CompressedTextureManager.Reset();

        OpaqueInstanceCount = 0;
        TransparentInstanceCount = 0;

        ColorChanged = false;
        ShapePropertyChanged = false;
        UsingCache = true;

        CurrentShapeIDs = PreviousShapeIDs[PreviousBufferID];
        CurrentInstanceData = PreviousInstanceData[PreviousBufferID];

        CurrentOpaqueDataSize = CurrentInstanceData[0].size();
        CurrentTransparentDataSize = CurrentInstanceData[1].size();
    }

    template <typename T>
    void Add(T *shape, uint16_t *TextureSize, unsigned char Channels);

    template <typename T>
    void Add(T *shape);

    void Render();
};
} // namespace PMMA::Internal::Rendering::Core2D