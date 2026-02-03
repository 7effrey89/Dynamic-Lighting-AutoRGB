#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

namespace winrt
{
    using namespace Windows::UI;
    using namespace Windows::Foundation;
    using namespace Windows::UI::Xaml;
    using namespace Microsoft::UI::Xaml;
}

namespace winrt::AutoRGBPrototype::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();

        m_graphicsManager.Initialize();
        m_rgbDeviceManager.Initialize();
    }

    void MainWindow::captureButton_Click(
        winrt::IInspectable const&,
        winrt::RoutedEventArgs const&)
    {
        if (m_graphicsManager.IsCaptureSupported())
        {
            Vector3 color;
            uint32_t histogramArray[NUM_BINS];

            // Subscribe to the CaptureTaken event raised by the GraphicsManager 
            m_graphicsManager.CaptureTaken([this, &color, &histogramArray](const auto&, const auto& args)
                {
                    Color c;
                    c.R = args.R();
                    c.G = args.G();
                    c.B = args.B();

                    colorOutput().Fill(winrt::Media::SolidColorBrush(c));

                    // Use zone colors if available
                    auto zoneColorsFlat = args.ZoneColorsFlat();
                    int32_t zoneCount = args.ZoneCount();
                    
                    if (zoneColorsFlat && zoneCount > 0 && zoneColorsFlat.Size() >= static_cast<uint32_t>(zoneCount * 3))
                    {
                        // Convert flat array to std::vector<RGBColor>
                        std::vector<RGBColor> colors;
                        colors.reserve(zoneCount);
                        
                        for (int32_t i = 0; i < zoneCount; ++i)
                        {
                            uint8_t r = zoneColorsFlat.GetAt(i * 3 + 0);
                            uint8_t g = zoneColorsFlat.GetAt(i * 3 + 1);
                            uint8_t b = zoneColorsFlat.GetAt(i * 3 + 2);
                            colors.emplace_back(255, r, g, b);
                        }

                        // Set per-lamp colors on the selected device
                        m_rgbDeviceManager.SetLampColors(colors);
                    }
                    else
                    {
                        // Fallback: change the color of all RGB devices to match the predominant color
                        m_rgbDeviceManager.ChangeColor(args.R(), args.G(), args.B());
                    }
                });

            m_graphicsManager.StartCapture();
        }
        else
        {
            buttonOutput().Text(L"Capture is not supported.");
        }
    }
}
