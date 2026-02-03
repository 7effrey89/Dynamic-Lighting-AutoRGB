#pragma once

#include "ZoneLayout.h"
#include "RGBDeviceManager.h"
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <algorithm>

// Extracts per-zone colors from captured frame using CPU sampling
class ZoneColorExtractor
{
public:
    ZoneColorExtractor() {}

    // Sample colors from zones in the texture
    // Returns a vector of RGB colors, one per zone
    std::vector<RGBColor> ExtractZoneColors(
        ID3D11DeviceContext* context,
        ID3D11Texture2D* texture,
        const std::vector<Zone>& zones,
        int screenWidth,
        int screenHeight)
    {
        std::vector<RGBColor> zoneColors;
        zoneColors.reserve(zones.size());

        // Create a staging texture for CPU readback
        D3D11_TEXTURE2D_DESC textureDesc;
        texture->GetDesc(&textureDesc);

        D3D11_TEXTURE2D_DESC stagingDesc = textureDesc;
        stagingDesc.Usage = D3D11_USAGE_STAGING;
        stagingDesc.BindFlags = 0;
        stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        stagingDesc.MiscFlags = 0;

        Microsoft::WRL::ComPtr<ID3D11Device> device;
        context->GetDevice(&device);

        Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTexture;
        HRESULT hr = device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
        if (FAILED(hr))
        {
            return zoneColors;
        }

        // Copy texture to staging
        context->CopyResource(stagingTexture.Get(), texture);

        // Map the staging texture
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        hr = context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
        if (FAILED(hr))
        {
            return zoneColors;
        }

        // Sample each zone
        for (const auto& zone : zones)
        {
            uint64_t totalR = 0, totalG = 0, totalB = 0;
            uint32_t pixelCount = 0;

            // Convert normalized coordinates to pixel coordinates
            int startX = static_cast<int>(zone.x * screenWidth);
            int startY = static_cast<int>(zone.y * screenHeight);
            int endX = static_cast<int>((zone.x + zone.width) * screenWidth);
            int endY = static_cast<int>((zone.y + zone.height) * screenHeight);

            // Clamp to screen bounds
            startX = (std::max)(0, (std::min)(startX, screenWidth - 1));
            startY = (std::max)(0, (std::min)(startY, screenHeight - 1));
            endX = (std::max)(0, (std::min)(endX, screenWidth));
            endY = (std::max)(0, (std::min)(endY, screenHeight));

            // Sample every Nth pixel for performance (stride)
            const int stride = 4; // Sample every 4th pixel in each direction

            for (int y = startY; y < endY; y += stride)
            {
                uint8_t* rowStart = static_cast<uint8_t*>(mappedResource.pData) + (y * mappedResource.RowPitch);
                
                for (int x = startX; x < endX; x += stride)
                {
                    // BGRA format
                    uint8_t b = rowStart[x * 4 + 0];
                    uint8_t g = rowStart[x * 4 + 1];
                    uint8_t r = rowStart[x * 4 + 2];
                    // uint8_t a = rowStart[x * 4 + 3]; // not used

                    totalR += r;
                    totalG += g;
                    totalB += b;
                    ++pixelCount;
                }
            }

            // Calculate average color for this zone
            if (pixelCount > 0)
            {
                uint32_t avgR = static_cast<uint32_t>(totalR / pixelCount);
                uint32_t avgG = static_cast<uint32_t>(totalG / pixelCount);
                uint32_t avgB = static_cast<uint32_t>(totalB / pixelCount);
                zoneColors.emplace_back(255, avgR, avgG, avgB);
            }
            else
            {
                // Fallback to black if no pixels sampled
                zoneColors.emplace_back(255, 0, 0, 0);
            }
        }

        context->Unmap(stagingTexture.Get(), 0);

        return zoneColors;
    }
};
