#pragma once

#include "ZoneConfiguration.h"
#include <vector>

// Represents a normalized rectangular zone (coordinates from 0.0 to 1.0)
struct Zone
{
    float x;      // Left edge (0.0 = left of screen, 1.0 = right of screen)
    float y;      // Top edge (0.0 = top of screen, 1.0 = bottom of screen)
    float width;  // Width (0.0 to 1.0)
    float height; // Height (0.0 to 1.0)

    Zone(float x_, float y_, float width_, float height_)
        : x(x_), y(y_), width(width_), height(height_)
    {
    }
};

// Manages the layout of edge zones for ambilight-style capture
class ZoneLayout
{
public:
    ZoneLayout() {}

    // Generate zones based on configuration
    // Zones are ordered clockwise starting from top-left:
    // 1. Top edge: left to right
    // 2. Right edge: top to bottom
    // 3. Bottom edge: right to left
    // 4. Left edge: bottom to top
    void GenerateZones(const ZoneConfiguration& config)
    {
        m_zones.clear();
        m_zones.reserve(config.GetTotalZoneCount());

        const float thickness = config.edgeThicknessPercent;

        // Top edge zones (left to right)
        if (config.topZoneCount > 0)
        {
            float zoneWidth = 1.0f / config.topZoneCount;
            for (uint32_t i = 0; i < config.topZoneCount; ++i)
            {
                float x = i * zoneWidth;
                m_zones.emplace_back(x, 0.0f, zoneWidth, thickness);
            }
        }

        // Right edge zones (top to bottom)
        if (config.rightZoneCount > 0)
        {
            float zoneHeight = 1.0f / config.rightZoneCount;
            for (uint32_t i = 0; i < config.rightZoneCount; ++i)
            {
                float y = i * zoneHeight;
                m_zones.emplace_back(1.0f - thickness, y, thickness, zoneHeight);
            }
        }

        // Bottom edge zones (right to left)
        if (config.bottomZoneCount > 0)
        {
            float zoneWidth = 1.0f / config.bottomZoneCount;
            for (uint32_t i = 0; i < config.bottomZoneCount; ++i)
            {
                float x = 1.0f - (i + 1) * zoneWidth;
                m_zones.emplace_back(x, 1.0f - thickness, zoneWidth, thickness);
            }
        }

        // Left edge zones (bottom to top)
        if (config.leftZoneCount > 0)
        {
            float zoneHeight = 1.0f / config.leftZoneCount;
            for (uint32_t i = 0; i < config.leftZoneCount; ++i)
            {
                float y = 1.0f - (i + 1) * zoneHeight;
                m_zones.emplace_back(0.0f, y, thickness, zoneHeight);
            }
        }
    }

    const std::vector<Zone>& GetZones() const { return m_zones; }
    size_t GetZoneCount() const { return m_zones.size(); }

private:
    std::vector<Zone> m_zones;
};
