# AutoRGB Sample Overview

### Description

This sample demonstrates how to extract zone-based ambient colors from a desktop 
screen edge and use them to illuminate individual LED lamps on a connected RGB 
device that fits the HID standard via the Windows.Devices.Lights API. The app 
uses an ambilight-style approach, sampling colors from thin bands along the screen 
edges and mapping them to individual lamps for an immersive lighting experience.

This sample requires the C++/WinRT, Windows App SDK, WinUI, and Windows 
Implementation Library packages. 

### Building the Sample
Deploy the app. Click on the "Capture screen" button to begin the screen
capture that calculates the zone-based ambient colors to display. Watch your 
lighting device illuminate with colors that match the edges of your screen! 
Try it on videos, games, and more. The app will display the calculated average 
color as well.

### Installing Pre-built Packages

Pre-built MSIX packages are available from GitHub Actions artifacts. For information 
about installing signed MSIX packages, certificate requirements, and troubleshooting, 
see the [MSIX Package Signing Guide](docs/signing.md).

**Note:** The sample app captures the monitor with the active window. If using 
a multi-monitor setup, the active window's monitor will be captured.

### Zone-Based Ambilight Capture

The app divides the screen edges into configurable zones and samples the average 
color from each zone. By default, the configuration is:
- **Top edge**: 6 zones (left to right)
- **Left edge**: 4 zones (bottom to top)
- **Right edge**: 4 zones (top to bottom)
- **Bottom edge**: 0 zones (disabled)

This creates 14 total zones arranged in a clockwise pattern around the screen 
perimeter, providing an ambilight-like effect.

#### Zone Ordering
Zones are ordered clockwise starting from the top-left:
1. **Top edge zones**: ordered left to right
2. **Right edge zones**: ordered top to bottom
3. **Bottom edge zones**: ordered right to left (if enabled)
4. **Left edge zones**: ordered bottom to top

#### Configuration Options
The zone capture can be customized via the `ZoneConfiguration` class:

- **Zone counts per edge**: `topZoneCount`, `leftZoneCount`, `rightZoneCount`, `bottomZoneCount`
  - Default: 6, 4, 4, 0 respectively
  
- **Edge thickness**: `edgeThicknessPercent`
  - Default: 0.03 (3% of screen dimension)
  - Controls how far from the edge to sample
  
- **Target FPS**: `targetFPS`
  - Default: 30 FPS
  - Throttles processing to maintain stable performance
  
- **Smoothing**: `smoothingAlpha`, `smoothingEnabled`
  - Default: alpha = 0.3, enabled = true
  - Uses exponential moving average to reduce flicker
  - Formula: `smoothed = prev + alpha * (current - prev)`
  - Lower alpha = smoother but slower response

### Device Selection and Per-Lamp Control

The app automatically selects the first discovered LampArray device and sets 
individual lamp colors based on the zone colors:

- **Auto-selection**: The first LampArray device found is automatically selected
- **Per-lamp mapping**: Zone colors are mapped to lamp indices (1:1 up to `min(zoneCount, lampCount)`)
- **Device removal handling**: If the selected device is removed, the app automatically selects the next available device

### Implementation Notes
There are 4 main components to this sample: screen capture, zone color extraction, 
color smoothing, and RGB device manager. 

##### Screen Capture
The screen capture is done via the Windows.Graphics.Capture API using D3D11. 
The monitor that contains the active window is the screen that will be captured. 
A screen capture is taken whenever a change is detected on the monitor display 
(e.g. resizing a window, video frame changes). This code is part of the 
GraphicsManager class. 

##### Zone Color Extraction
The `ZoneColorExtractor` samples the average color from each defined zone using 
CPU-based texture readback. For performance, it samples every 4th pixel within 
each zone. The `ZoneLayout` class generates normalized rectangular zones based 
on the configuration.

##### Color Smoothing
The `ZoneColorSmoother` applies exponential moving average smoothing to each 
zone's color independently. This reduces flickering and creates a more pleasant 
visual effect. Smoothing can be disabled or tuned via the configuration.

##### RGB Device Manager
The `RGBDeviceManager` class listens for when HID-compliant LampArray devices 
are added or removed. It automatically selects the first device and uses 
`SetColorForIndex()` to set individual lamp colors. When the MainWindow class 
receives zone colors from the GraphicsManager, it passes them to the 
RGBDeviceManager to update the selected device.

##### FPS Throttling
The GraphicsManager throttles frame processing to the target FPS (default 30) 
to maintain stable performance and reduce CPU/GPU load. Frames are skipped if 
insufficient time has passed since the last processed frame.

### Backward Compatibility

The app maintains backward compatibility with single-color mode. The 
`CaptureTakenEventArgs` includes both individual zone colors and an average 
color calculated from all zones. The original histogram-based color algorithm 
is still available (set `m_useZoneCapture = false` in GraphicsManager).

### Additional Notes
This sample was created in collaboration with the Xbox team. 

### Resources and Links
Learn more about Windows App SDK here:
https://docs.microsoft.com/windows/apps/windows-app-sdk/
Learn more about WinUI3 here:
https://docs.microsoft.com/windows/apps/winui/winui3/
Learn more about C++/WinRT here:
http://aka.ms/cppwinrt/
Learn more about Windows.Devices.Lights here: 
https://learn.microsoft.com/en-us/uwp/api/windows.devices.lights?view=winrt-22621
Learn more about Windows.Graphics.Capture here:
https://learn.microsoft.com/en-us/windows/uwp/audio-video-camera/screen-capture

