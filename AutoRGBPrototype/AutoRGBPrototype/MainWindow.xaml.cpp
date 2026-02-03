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
                    auto zoneColors = args.ZoneColors();
                    if (zoneColors && zoneColors.Size() > 0)
                    {
                        // Convert to std::vector<RGBColor> for device manager
                        std::vector<RGBColor> colors;
                        colors.reserve(zoneColors.Size());
                        for (const auto& zc : zoneColors)
                        {
                            colors.emplace_back(255, zc.R, zc.G, zc.B);
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
