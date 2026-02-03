# Zone-Based Ambilight Implementation - Testing Guide

## Overview
This implementation adds zone-based edge color capture with per-lamp control to the AutoRGB application. The app now samples colors from thin bands along screen edges and maps them to individual lamps on a selected LampArray device.

## Key Changes Summary

### New Files Added
1. **ZoneConfiguration.h** - Configuration structure for zone settings
2. **ZoneLayout.h** - Generates normalized rectangular zones for each edge
3. **ZoneColorExtractor.h** - Extracts per-zone colors from captured frame using CPU sampling
4. **ZoneColorSmoother.h** - Applies exponential moving average smoothing per zone

### Modified Files
1. **GraphicsManager.idl** - Extended CaptureTakenEventArgs to include ZoneColor vector
2. **GraphicsManager.h/.cpp** - Integrated zone capture, smoothing, and FPS throttling
3. **RGBDeviceManager.h/.cpp** - Added device selection and per-lamp color control
4. **MainWindow.xaml.cpp** - Updated to use zone colors when available
5. **AutoRGBPrototype.vcxproj** - Added new header files to build

## Default Configuration
- **Top edge**: 6 zones (left to right)
- **Left edge**: 4 zones (bottom to top)
- **Right edge**: 4 zones (top to bottom)
- **Bottom edge**: 0 zones (disabled)
- **Total zones**: 14
- **Edge thickness**: 3% of screen dimension
- **Target FPS**: 30
- **Smoothing alpha**: 0.3 (enabled)

## Zone Ordering
Zones are ordered clockwise starting from top-left:
1. Top edge zones (0-5): left to right
2. Right edge zones (6-9): top to bottom
3. Bottom edge zones: right to left (if enabled)
4. Left edge zones (10-13): bottom to top

## Testing Instructions

### Prerequisites
- Windows 10/11 with Dynamic Lighting support
- Visual Studio 2022 with C++ and UWP development tools
- At least one LampArray-compatible RGB device connected

### Build Steps
1. Open `AutoRGBPrototype.sln` in Visual Studio 2022
2. Select x64 or ARM64 platform (depending on your system)
3. Build in Debug or Release configuration
4. Deploy to local machine

### Test Scenarios

#### Test 1: Basic Zone Capture
1. Launch the application
2. Click "Capture screen" button
3. **Expected**: Device should light up with 14 different colors corresponding to screen edges
4. **Verify**: Colors update dynamically as you move content on screen

#### Test 2: Smoothing
1. Play a video with rapidly changing colors
2. **Expected**: Transitions should be smooth, not flickering
3. To test without smoothing: Set `m_zoneConfig.smoothingEnabled = false` in GraphicsManager::Initialize()

#### Test 3: FPS Throttling
1. Monitor CPU usage during capture
2. **Expected**: Processing should maintain ~30 FPS, not run unthrottled
3. To verify: Add logging in RunShader() to print frame timestamps

#### Test 4: Device Selection
1. Connect multiple LampArray devices
2. Launch app and click "Capture screen"
3. **Expected**: First device discovered should be selected and receive colors
4. Disconnect the selected device
5. **Expected**: App should auto-select next available device

#### Test 5: Zone-to-Lamp Mapping
1. With a device having fewer lamps than zones (< 14):
   - **Expected**: Only `min(lampCount, 14)` lamps are set
2. With a device having more lamps than zones (> 14):
   - **Expected**: First 14 lamps receive zone colors

#### Test 6: Backward Compatibility
1. Set `m_useZoneCapture = false` in GraphicsManager.h
2. Rebuild and run
3. **Expected**: App should fall back to single-color mode (all lamps same color)

### Known Limitations
1. **CPU sampling performance**: The ZoneColorExtractor uses CPU readback which may be slower than a GPU compute shader approach. If performance is insufficient, consider implementing a zone-specific compute shader.
2. **Capture requires active window**: The screen capture still requires the app to have an active window, though it doesn't need to be foreground.
3. **Configuration persistence**: Settings are currently hard-coded in ZoneConfiguration defaults. To persist settings, integrate with Windows ApplicationData.

### Troubleshooting

**Issue**: Build fails with "Cannot find ZoneConfiguration.h"
- **Solution**: Ensure all new .h files are added to the vcxproj file (already done in this commit)

**Issue**: IDL compilation errors about ZoneColor
- **Solution**: Clean and rebuild the solution to regenerate IDL-generated files

**Issue**: Device doesn't light up
- **Solution**: 
  - Check Windows Dynamic Lighting is enabled in Settings
  - Verify device is LampArray-compatible
  - Check device manager for proper driver installation

**Issue**: Colors seem incorrect or washed out
- **Solution**: 
  - Verify edge thickness setting (3% default)
  - Check smoothing alpha (lower = smoother but less responsive)
  - Ensure proper zone ordering in testing

### Performance Notes
- **FPS throttling**: Default 30 FPS provides good balance of responsiveness and performance
- **Sampling stride**: ZoneColorExtractor samples every 4th pixel for performance. Adjust in ZoneColorExtractor.h if needed
- **Memory usage**: Per-zone smoothing maintains previous colors, minimal memory overhead

### Configuration Customization
To customize zone configuration, modify values in `GraphicsManager::Initialize()`:

```cpp
// Example: More zones, thinner edges, faster response
m_zoneConfig.topZoneCount = 10;
m_zoneConfig.leftZoneCount = 6;
m_zoneConfig.rightZoneCount = 6;
m_zoneConfig.bottomZoneCount = 4;
m_zoneConfig.edgeThicknessPercent = 0.02f;  // 2%
m_zoneConfig.smoothingAlpha = 0.5f;         // Faster response
m_zoneConfig.targetFPS = 60;                // Higher frame rate
```

## Success Criteria Checklist
- [ ] App builds successfully on Windows
- [ ] Default configuration produces 14 zone colors
- [ ] Colors update dynamically with screen content
- [ ] Smoothing reduces flicker
- [ ] FPS throttling maintains stable 30 FPS
- [ ] First device is auto-selected on startup
- [ ] Device removal triggers auto-selection of next device
- [ ] Per-lamp colors are set correctly (visible when testing with colorful content)

## Next Steps (Future Enhancements)
1. Implement GPU compute shader for zone extraction (better performance)
2. Add UI controls for configuration (zone counts, smoothing, FPS)
3. Implement settings persistence
4. Add device selection dropdown in UI
5. Support for capturing when app is not foreground
6. Color calibration per device
