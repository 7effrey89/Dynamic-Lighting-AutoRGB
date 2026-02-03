#pragma once

#include "GraphicsManager.g.h"
#include "CaptureTakenEventArgs.g.h"

#include "ShaderDefines.h"
#include "ColorAlgorithm.h"
#include "ZoneConfiguration.h"
#include "ZoneLayout.h"
#include "ZoneColorExtractor.h"
#include "ZoneColorSmoother.h"

#include <Vector3.h>
#include <future>
#include <chrono>

namespace winrt::AutoRGBPrototype::implementation
{
    struct GraphicsManager : GraphicsManagerT<GraphicsManager>
    {
    public:
        GraphicsManager() {}
        ~GraphicsManager() {}

        void Initialize();

        bool IsCaptureSupported() { return m_isCaptureSupported; }
        int GetNumPixels() { return m_numPixels; }

        void StartCapture();

        // Event delegates
        winrt::event_token CaptureTaken(winrt::Windows::Foundation::TypedEventHandler<AutoRGBPrototype::GraphicsManager, AutoRGBPrototype::CaptureTakenEventArgs> const& handler);
        void CaptureTaken(winrt::event_token const& token) noexcept;

    private:
        void GetDisplayInfo();
        void CreateDevices();
        void CreateShaderResources();

        std::future<void> TakeScreenCapture();
        std::future<void> TakeScreenCaptureAsync();
        void RunShader();

        uint32_t ComputePaddedBufferSize(size_t size);

    private:
        // D3D devices 
        winrt::com_ptr<ID3D11Device> m_d3dDevice;
        winrt::com_ptr<IDXGIDevice> m_dxgiDevice;
        winrt::com_ptr<ID3D11DeviceContext> m_d3dDeviceContext;

        // Screen capture resources 
        HMONITOR m_display;
        int m_height = 0;
        int m_width = 0;
        int m_numPixels = 0;
        bool m_isCaptureSupported = false;
        winrt::Windows::Graphics::Capture::GraphicsCaptureSession m_session{ nullptr };
        winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool m_framePool{ nullptr };
        winrt::event<winrt::Windows::Foundation::TypedEventHandler<AutoRGBPrototype::GraphicsManager, AutoRGBPrototype::CaptureTakenEventArgs>> m_captureTaken;

        // Shader resources
        winrt::com_ptr<ID3D11Texture2D> m_captureTexture;
        winrt::com_ptr<ID3D11Buffer> m_constantBuffer;
        winrt::com_ptr<ID3D11Buffer> m_outputBuffer;
        winrt::com_ptr<ID3D11Buffer> m_readbackBuffer;
        winrt::com_ptr<ID3D11ShaderResourceView> m_captureSrv;
        winrt::com_ptr<ID3D11UnorderedAccessView> m_outputUav;
        winrt::com_ptr<ID3D11ComputeShader> m_shader;

        // Predominant color calculator
        ColorAlgorithm m_colorAlgorithm;

        // Zone-based capture
        ZoneConfiguration m_zoneConfig;
        ZoneLayout m_zoneLayout;
        ZoneColorExtractor m_zoneColorExtractor;
        ZoneColorSmoother m_zoneColorSmoother;
        bool m_useZoneCapture = true;

        // FPS throttling
        std::chrono::steady_clock::time_point m_lastFrameTime;
        std::chrono::milliseconds m_frameInterval{ 33 }; // ~30 FPS default
    };

    struct CaptureTakenEventArgs : CaptureTakenEventArgsT<CaptureTakenEventArgs>
    {
        CaptureTakenEventArgs(uint8_t r, uint8_t g, uint8_t b) : m_R(r), m_G(g), m_B(b)
        {
        }

        CaptureTakenEventArgs(uint8_t r, uint8_t g, uint8_t b, 
            winrt::Windows::Foundation::Collections::IVector<AutoRGBPrototype::ZoneColor> zoneColors)
            : m_R(r), m_G(g), m_B(b), m_zoneColors(zoneColors)
        {
        }

        uint8_t R()
        {
            return m_R;
        }

        uint8_t G()
        {
            return m_G;
        }

        uint8_t B()
        {
            return m_B;
        }

        winrt::Windows::Foundation::Collections::IVector<AutoRGBPrototype::ZoneColor> ZoneColors()
        {
            return m_zoneColors;
        }

    private:
        uint8_t m_R;
        uint8_t m_G;
        uint8_t m_B;
        winrt::Windows::Foundation::Collections::IVector<AutoRGBPrototype::ZoneColor> m_zoneColors{ nullptr };
    };

    // This struct is needed to pass in information about the monitor size to the shader
    struct ScreenWidth
    {
        uint32_t Width;
    };
}
