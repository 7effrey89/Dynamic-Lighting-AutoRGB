#pragma once

#include <cstdint>

// Configuration for zone-based edge capture
struct ZoneConfiguration
{
    // Number of zones per edge
    uint32_t topZoneCount = 6;
    uint32_t leftZoneCount = 4;
    uint32_t rightZoneCount = 4;
    uint32_t bottomZoneCount = 0;

    // Edge thickness as percentage of screen dimension (0.0 to 1.0)
    float edgeThicknessPercent = 0.03f;

    // Target frames per second
    uint32_t targetFPS = 30;

    // Smoothing alpha (0.0 = no smoothing, 1.0 = instant change)
    float smoothingAlpha = 0.3f;

    // Whether smoothing is enabled
    bool smoothingEnabled = true;

    // Get total number of zones
    uint32_t GetTotalZoneCount() const
    {
        return topZoneCount + leftZoneCount + rightZoneCount + bottomZoneCount;
    }
};
