#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <bgfx/bgfx.h>

#include "Internal/Internal.hpp"
#include "Internal/Rendering/Core2D/Base.hpp"

namespace PMMA::Internal::Rendering::Core2D {
class GeneratedTextureInstance { // makes texture atlas for a RenderPipelineInstance
private:
    std::map<uintptr_t, PMMA::Internal::Rendering::Core2D::GeneratedTextureProperty *> RegisteredTextures;
    std::vector<PMMA::Internal::Rendering::Core2D::GeneratedTextureProperty *> PendingTextures;

    std::vector<PMMA::Internal::Rendering::Core2D::SkylineNode> Skyline;
    std::map<uintptr_t, PMMA::Internal::Rendering::Core2D::AtlasAllocation> Allocations;

    std::vector<unsigned char> AtlasPixels;

public:
    bool Dirty = false;

    bgfx::TextureHandle TextureHandle = BGFX_INVALID_HANDLE;

    uint32_t m_TextureWidth = 0;
    uint32_t m_TextureHeight = 0;
    uint32_t MaxTextureDimension = 1024;
    uintptr_t RenderPipelineInstanceID;
    bool Transparent = false;

    ~GeneratedTextureInstance() {
        if (bgfx::isValid(TextureHandle)) {
            bgfx::destroy(TextureHandle);
        }
    }

    bool FindPosition(
        uint32_t Width,
        uint32_t Height,
        uint32_t &OutX,
        uint32_t &OutY,
        size_t &OutSkylineIndex) {

        std::cout
            << "FindPosition "
            << Width << "x" << Height
            << " skyline nodes: "
            << Skyline.size()
            << std::endl;

        uint32_t BestY = UINT32_MAX;
        uint32_t BestX = UINT32_MAX;
        size_t BestIndex = SIZE_MAX;

        for (size_t i = 0; i < Skyline.size(); ++i) {
            uint32_t CandidateY;

            //
            // Check if the rectangle fits starting
            // at this skyline node.
            //
            uint32_t x = Skyline[i].x;

            if (x + Width > MaxTextureDimension)
                continue;

            uint32_t WidthRemaining = Width;
            uint32_t y = Skyline[i].y;

            size_t NodeIndex = i;

            while (WidthRemaining > 0) {
                //
                // The rectangle must sit above the
                // tallest skyline section it overlaps.
                //
                y = std::max(
                    y,
                    Skyline[NodeIndex].y);

                //
                // Would this exceed atlas height?
                //
                if (y + Height > MaxTextureDimension) {
                    break;
                }

                if (Skyline[NodeIndex].width >= WidthRemaining) {
                    WidthRemaining = 0;
                } else {
                    WidthRemaining -= Skyline[NodeIndex].width;
                }

                ++NodeIndex;

                //
                // Ran out of skyline before fitting.
                //
                if (NodeIndex >= Skyline.size() && WidthRemaining > 0) {
                    break;
                }
            }

            //
            // Failed to fit.
            //
            if (WidthRemaining > 0)
                continue;

            CandidateY = y;

            //
            // Bottom-left heuristic:
            // lower Y wins, then lower X.
            //
            if (CandidateY < BestY ||
                (CandidateY == BestY &&
                 x < BestX)) {
                BestY = CandidateY;
                BestX = x;
                BestIndex = i;
            }
        }

        if (BestIndex == SIZE_MAX) {
            return false;
        }

        OutX = BestX;
        OutY = BestY;
        OutSkylineIndex = BestIndex;

        return true;
    }

    void InsertSkylineLevel(
        size_t Index,
        uint32_t X,
        uint32_t Y,
        uint32_t Width,
        uint32_t Height) {
        SkylineNode NewNode{
            X,
            Y + Height,
            Width};

        //
        // Insert the new skyline segment.
        //
        Skyline.insert(
            Skyline.begin() + Index,
            NewNode);

        //
        // Remove or shrink skyline nodes that
        // are now covered by this rectangle.
        //
        for (size_t i = Index + 1;
             i < Skyline.size();) {
            SkylineNode &Current = Skyline[i];
            SkylineNode &Previous = Skyline[i - 1];

            uint32_t PreviousEnd =
                Previous.x + Previous.width;

            //
            // This node is completely outside the
            // inserted rectangle.
            //
            if (Current.x >= PreviousEnd) {
                break;
            }

            uint32_t Shrink =
                PreviousEnd - Current.x;

            //
            // The new rectangle completely covers
            // this skyline node.
            //
            if (Shrink >= Current.width) {
                Skyline.erase(
                    Skyline.begin() + i);

                continue;
            }

            //
            // The new rectangle partially overlaps
            // this node.
            //
            Current.x += Shrink;
            Current.width -= Shrink;

            break;
        }
    }

    void MergeSkyline() {
        if (Skyline.size() < 2)
            return;

        for (size_t i = 0; i < Skyline.size() - 1;) {
            SkylineNode &Current = Skyline[i];
            SkylineNode &Next = Skyline[i + 1];

            //
            // Adjacent nodes at the same height
            // can be represented as one node.
            //
            if (Current.y == Next.y) {
                Current.width += Next.width;

                Skyline.erase(
                    Skyline.begin() + i + 1);

                //
                // Do not increment i here.
                // The newly expanded node may also
                // merge with the following node.
                //
                continue;
            }

            ++i;
        }
    }

    void RegisterTexture(PMMA::Internal::Rendering::Core2D::GeneratedTextureProperty *Texture) {
        if (Skyline.empty()) {
            Skyline.push_back({0,
                               0,
                               MaxTextureDimension});
        }

        if (Texture == nullptr) {
            return;
        }

        if (RegisteredTextures.find(Texture->ID) != RegisteredTextures.end()) {
            return;
        }

        constexpr uint32_t Padding = 16;

        uint32_t PackedWidth =
            Texture->TextureSize[0] + Padding * 2;

        uint32_t PackedHeight =
            Texture->TextureSize[1] + Padding * 2;

        uint32_t X;
        uint32_t Y;
        size_t SkylineIndex;

        //
        // Attempt to allocate space.
        //
        if (!FindPosition(
                PackedWidth,
                PackedHeight,
                X,
                Y,
                SkylineIndex)) {
            // Atlas full.
            // Later this can trigger atlas growth.
            std::cout << "Atlas full" << std::endl;
            return;
        }

        //
        // Reserve space immediately.
        //
        InsertSkylineLevel(
            SkylineIndex,
            X,
            Y,
            PackedWidth,
            PackedHeight);

        MergeSkyline();

        Allocations[Texture->ID] =
            {
                X,
                Y,
                PackedWidth,
                PackedHeight};

        //
        // Register texture.
        //
        RegisteredTextures.emplace(
            Texture->ID,
            Texture);

        ++Texture->References;

        //
        // Store actual usable texture location.
        //
        auto &Location =
            Texture->RegisteredRenderPipelineInstances
                [RenderPipelineInstanceID];

        Location[0] = X + Padding;
        Location[1] = Y + Padding;

        PendingTextures.push_back(Texture);
        Dirty = true;
    }

    // Here, if the texture atlas is dirty, the texture atlas should be generated using the properties from 'RegisteredTextures' and written to a BGFX texture.
    void Assemble() {
        char Channels = 3;
        if (Transparent) {
            Channels++;
        }

        //
        // Find required atlas dimensions.
        //
        m_TextureWidth = 1;
        m_TextureHeight = 1;

        for (auto &[ID, Allocation] : Allocations) {
            m_TextureWidth =
                std::max(
                    m_TextureWidth,
                    Allocation.X + Allocation.Width);

            m_TextureHeight =
                std::max(
                    m_TextureHeight,
                    Allocation.Y + Allocation.Height);
        }

        //
        // Round atlas dimensions up to power-of-two.
        //
        auto NextPowerOfTwo =
            [](uint32_t Value) {
                uint32_t Result = 1;

                while (Result < Value)
                    Result <<= 1;

                return Result;
            };

        m_TextureWidth =
            NextPowerOfTwo(m_TextureWidth);

        m_TextureHeight =
            NextPowerOfTwo(m_TextureHeight);

        size_t RequiredSize =
            m_TextureWidth *
            m_TextureHeight *
            Channels;

        if (AtlasPixels.size() != RequiredSize) {
            AtlasPixels.resize(
                RequiredSize,
                255);
        }

        //
        // Copy textures into their assigned regions.
        //
        for (auto *Texture : PendingTextures) {
            auto Allocation =
                Allocations[Texture->ID];

            auto Position =
                Texture->RegisteredRenderPipelineInstances
                    [RenderPipelineInstanceID];

            uint32_t DestX = Position[0];
            uint32_t DestY = Position[1];

            for (uint32_t Y = 0;
                 Y < Texture->TextureSize[1];
                 ++Y) {
                for (uint32_t X = 0;
                     X < Texture->TextureSize[0];
                     ++X) {
                    size_t SourceIndex =
                        (Y * Texture->TextureSize[0] + X) * Channels;

                    size_t DestinationIndex =
                        ((DestY + Y) * m_TextureWidth +
                         (DestX + X)) *
                        Channels;

                    AtlasPixels[DestinationIndex + 0] =
                        Texture->PixelData[SourceIndex + 0];

                    AtlasPixels[DestinationIndex + 1] =
                        Channels > 1
                            ? Texture->PixelData[SourceIndex + 1]
                            : 255;

                    AtlasPixels[DestinationIndex + 2] =
                        Channels > 2
                            ? Texture->PixelData[SourceIndex + 2]
                            : 255;

                    AtlasPixels[DestinationIndex + 3] =
                        Channels > 3
                            ? Texture->PixelData[SourceIndex + 3]
                            : 255;
                }
            }
        }

        //
        // Replace BGFX texture.
        //
        if (bgfx::isValid(TextureHandle)) {
            bgfx::destroy(TextureHandle);
        }

        if (Transparent) {
            TextureHandle =
                bgfx::createTexture2D(
                    static_cast<uint16_t>(m_TextureWidth),
                    static_cast<uint16_t>(m_TextureHeight),
                    false,
                    1,
                    bgfx::TextureFormat::RGBA8,
                    BGFX_TEXTURE_NONE,
                    bgfx::copy(
                        AtlasPixels.data(),
                        static_cast<uint32_t>(AtlasPixels.size())));
        } else {
            TextureHandle =
                bgfx::createTexture2D(
                    static_cast<uint16_t>(m_TextureWidth),
                    static_cast<uint16_t>(m_TextureHeight),
                    false,
                    1,
                    bgfx::TextureFormat::RGB8,
                    BGFX_TEXTURE_NONE,
                    bgfx::copy(
                        AtlasPixels.data(),
                        static_cast<uint32_t>(AtlasPixels.size())));
        }

        PendingTextures.clear();
        Dirty = false;
    }
};
} // namespace PMMA::Internal::Rendering::Core2D