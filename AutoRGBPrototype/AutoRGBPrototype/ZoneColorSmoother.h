#pragma once

#include "RGBDeviceManager.h"
#include <vector>

// Applies exponential moving average smoothing to zone colors
class ZoneColorSmoother
{
public:
    ZoneColorSmoother() {}

    // Initialize or resize the smoother for the given number of zones
    void Initialize(size_t zoneCount)
    {
        m_previousColors.clear();
        m_previousColors.resize(zoneCount, RGBColor(255, 0, 0, 0));
        m_initialized = false;
    }

    // Apply smoothing to zone colors
    // Returns smoothed colors
    std::vector<RGBColor> SmoothColors(
        const std::vector<RGBColor>& currentColors,
        float alpha,
        bool enabled)
    {
        // If smoothing is disabled or this is the first frame, return current colors
        if (!enabled || !m_initialized)
        {
            m_previousColors = currentColors;
            m_initialized = true;
            return currentColors;
        }

        // Ensure we have the right size
        if (m_previousColors.size() != currentColors.size())
        {
            Initialize(currentColors.size());
            m_previousColors = currentColors;
            return currentColors;
        }

        std::vector<RGBColor> smoothedColors;
        smoothedColors.reserve(currentColors.size());

        // Apply exponential moving average: smoothed = prev + alpha * (current - prev)
        for (size_t i = 0; i < currentColors.size(); ++i)
        {
            const auto& prev = m_previousColors[i];
            const auto& curr = currentColors[i];

            uint32_t smoothedR = static_cast<uint32_t>(prev.r + alpha * (static_cast<float>(curr.r) - static_cast<float>(prev.r)));
            uint32_t smoothedG = static_cast<uint32_t>(prev.g + alpha * (static_cast<float>(curr.g) - static_cast<float>(prev.g)));
            uint32_t smoothedB = static_cast<uint32_t>(prev.b + alpha * (static_cast<float>(curr.b) - static_cast<float>(prev.b)));

            // Clamp to 0-255
            smoothedR = min(255u, smoothedR);
            smoothedG = min(255u, smoothedG);
            smoothedB = min(255u, smoothedB);

            smoothedColors.emplace_back(255, smoothedR, smoothedG, smoothedB);
        }

        // Store for next iteration
        m_previousColors = smoothedColors;

        return smoothedColors;
    }

private:
    std::vector<RGBColor> m_previousColors;
    bool m_initialized = false;
};
