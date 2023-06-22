#pragma once

#include <concrt.h>
#include <d2d1_2.h>
#include <d3d11_2.h>
#include "BaseView.g.h"
#include <winrt/Windows.Graphics.Display.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.UI.Xaml.Media.h>


namespace winrt::SwapChainPanelBugRepro::implementation
{
  struct BaseView : BaseViewT<BaseView>
  {
  protected:
    BaseView();
    ~BaseView();

    virtual void CreateDeviceIndependentResources();
    virtual void CreateDeviceResources();
    virtual void CreateSizeDependentResources();

    virtual void OnDeviceLost();

    virtual void OnSizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& e);
    virtual void OnCompositionScaleChanged(winrt::Microsoft::UI::Xaml::Controls::SwapChainPanel const& sender, winrt::Windows::Foundation::IInspectable const& args);

    virtual winrt::Windows::Foundation::IAsyncAction OnLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    virtual winrt::Windows::Foundation::IAsyncAction OnUnloaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    virtual winrt::Windows::Foundation::IAsyncAction AfterLoaded();
    virtual winrt::Windows::Foundation::IAsyncAction BeforeUnloaded();

    virtual void Present();

    static ::DirectX::XMFLOAT3 RgbFromBrush(winrt::Microsoft::UI::Xaml::Media::SolidColorBrush const& brush);
    static ::DirectX::XMFLOAT4 RgbaFromBrush(winrt::Microsoft::UI::Xaml::Media::SolidColorBrush const& brush);

    winrt::com_ptr<ID3D11Device1>          _d3dDevice;
    winrt::com_ptr<ID3D11DeviceContext1>   _d3dContext;
    winrt::com_ptr<ID3D11RenderTargetView> _d3dRenderTargetView;
    winrt::com_ptr<ID3D11DepthStencilView> _d3dDepthStencilView;
    winrt::com_ptr<ID3D11Texture2D>        _d3dBackBuffer;
    winrt::com_ptr<ID3D11Texture2D>        _d3dBackBufferMultiSampled;
    winrt::com_ptr<IDXGIFactory2>          _dxgiFactory;
    winrt::com_ptr<IDXGIAdapter>           _dxgiAdapter;
    winrt::com_ptr<IDXGIOutput>            _dxgiOutput;
    winrt::com_ptr<IDXGISwapChain2>        _dxgiSwapChain;
    winrt::com_ptr<ID2D1Factory1>          _d2dFactory;
    winrt::com_ptr<ID2D1Device>            _d2dDevice;
    winrt::com_ptr<ID2D1DeviceContext>     _d2dContext;
    winrt::com_ptr<ID2D1Bitmap1>           _d2dTargetBitmap;

    winrt::event_token _sizeChangedEventToken;
    winrt::event_token _compositionScaleChangedEventToken;
    winrt::event_token _loadedEventToken;
    winrt::event_token _unloadedEventToken;

    concurrency::critical_section _criticalSection;

    UINT _multisampleAntiAliasingCount   = 4;
    UINT _multisampleAntiAliasingQuality = 0;

    // Device Independent Pixels (DIPs) per inch. A DIP is defined as 1/96 of a logical inch.
    // See https://learn.microsoft.com/en-us/windows/win32/learnwin32/dpi-and-device-independent-pixels
    static constexpr float                   DipsPerInch = 96.0f;
    static constexpr DXGI_FORMAT             DxgiFormat  = DXGI_FORMAT_B8G8R8A8_UNORM;
    static constexpr DXGI_PRESENT_PARAMETERS PresentationParameters{};
    static constexpr D2D1_FACTORY_OPTIONS    FactoryOptions{};

    static constexpr UINT DeviceCreationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    static constexpr D3D_FEATURE_LEVEL FeatureLevels[] =
    {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0,
      D3D_FEATURE_LEVEL_9_3,
      D3D_FEATURE_LEVEL_9_2,
      D3D_FEATURE_LEVEL_9_1
    };

    float _renderTargetHeight;
    float _renderTargetWidth;

    float _compositionScaleX = 1.0f;
    float _compositionScaleY = 1.0f;

  private:
    void DestroyDeviceResources();
    void DestroySizeDependentResources();
    void CreateD3D11Device();
    void CreateD2D1Device();
    void CalculateRenderTargetSize();
    void ExtractMultisamplingParameters();
    void CreateOrResizeSwapChain();
    void CreateSwapChain();
    void ScaleSwapChain();
    void ObtainBackBuffer();
    void CreateRenderTargetView(ID3D11Resource* targetBuffer, const CD3D11_RENDER_TARGET_VIEW_DESC* renderTargetViewDesc);
    void CreateDepthStencilView(UINT count, D3D11_DSV_DIMENSION dsvDimension);
    void CreateMultiSampledBackBuffer();
    void SetD2D1Target(IDXGISurface* targetSurface);
    void DownsampleMultiSampleBackBufferToBackBuffer();
    void HandleResult(HRESULT result);
  };
}
