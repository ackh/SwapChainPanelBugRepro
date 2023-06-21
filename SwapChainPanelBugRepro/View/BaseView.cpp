#include "pch.h"
#include "BaseView.h"
#include "BaseView.g.cpp"


namespace winrt::SwapChainPanelBugRepro::implementation
{
  using namespace winrt::Microsoft::UI::Xaml;
  using namespace winrt::Microsoft::UI::Xaml::Controls;
  using namespace winrt::Microsoft::UI::Xaml::Media;
  using namespace winrt::Windows::Foundation;
  using namespace winrt::Windows::UI;


  BaseView::BaseView()
  {
    _loadedEventToken   = Loaded({ this, &BaseView::OnLoaded });
    _unloadedEventToken = Unloaded({ this, &BaseView::OnUnloaded });
  }


  BaseView::~BaseView()
  {
    Unloaded(_unloadedEventToken);
    Loaded(_loadedEventToken);
  }


  void BaseView::CreateDeviceIndependentResources()
  {
    winrt::check_hresult(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, FactoryOptions, _d2dFactory.put()));
    winrt::check_hresult(CreateDXGIFactory1(IID_PPV_ARGS(_dxgiFactory.put())));
    winrt::check_hresult(_dxgiFactory->EnumAdapters(0, _dxgiAdapter.put()));
    winrt::check_hresult(_dxgiAdapter->EnumOutputs(0, _dxgiOutput.put()));
  }


  void BaseView::CreateDeviceResources()
  {
    CreateD3D11Device();
    CreateD2D1Device();
    ExtractMultisamplingParameters();
  }


  void BaseView::CreateSizeDependentResources()
  {
    CalculateRenderTargetSize();
    CreateOrResizeSwapChain();
    ScaleSwapChain();
    ObtainBackBuffer();

    if(_multisampleAntiAliasingQuality <= 0)
    {
      CreateRenderTargetView(_d3dBackBuffer.get(), nullptr);
      CreateDepthStencilView(1, D3D11_DSV_DIMENSION_TEXTURE2D);
      SetD2D1Target(_d3dBackBuffer.as<IDXGISurface2>().get());
    }

    else
    {
      CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2DMS);
      CreateMultiSampledBackBuffer();
      CreateRenderTargetView(_d3dBackBufferMultiSampled.get(), &renderTargetViewDesc);
      CreateDepthStencilView(_multisampleAntiAliasingCount, D3D11_DSV_DIMENSION_TEXTURE2DMS);
      SetD2D1Target(_d3dBackBufferMultiSampled.as<IDXGISurface2>().get());
    }
  }


  void BaseView::OnDeviceLost()
  {
    _dxgiSwapChain = nullptr;
    DestroySizeDependentResources();
    DestroyDeviceResources();
    CreateDeviceResources();
    CreateSizeDependentResources();
  }


  void BaseView::OnSizeChanged(IInspectable const&, SizeChangedEventArgs const&)
  {
    Concurrency::critical_section::scoped_lock lock(_criticalSection);
    DestroySizeDependentResources();
    CreateSizeDependentResources();
  }


  void BaseView::OnCompositionScaleChanged(SwapChainPanel const&, IInspectable const&)
  {
    float compositionScaleX = CompositionScaleX();
    float compositionScaleY = CompositionScaleY();

    if(_compositionScaleX != compositionScaleX || _compositionScaleY != compositionScaleY)
    {
      Concurrency::critical_section::scoped_lock lock(_criticalSection);
      _compositionScaleX = compositionScaleX;
      _compositionScaleY = compositionScaleY;
      DestroySizeDependentResources();
      CreateSizeDependentResources();
    }
  }


  IAsyncAction BaseView::OnLoaded(IInspectable const&, RoutedEventArgs const&)
  {
    // Obtaining a strong reference ensures that the instance is kept alive
    // even while this coroutine is suspended. For more details see
    // https://learn.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/weak-references#safely-accessing-the-this-pointer-in-a-class-member-coroutine
    winrt::com_ptr<BaseView> keepInstanceAlive = get_strong();
    _sizeChangedEventToken                     = SizeChanged({ this, &BaseView::OnSizeChanged });
    _compositionScaleChangedEventToken         = CompositionScaleChanged({ this, &BaseView::OnCompositionScaleChanged });
    co_await AfterLoaded();
    co_return;
  }


  IAsyncAction BaseView::AfterLoaded()
  {
    co_return;
  }


  IAsyncAction BaseView::OnUnloaded(IInspectable const&, RoutedEventArgs const&)
  {
    // Obtaining a strong reference ensures that the instance is kept alive
    // even while this coroutine is suspended. For more details see
    // https://learn.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/weak-references#safely-accessing-the-this-pointer-in-a-class-member-coroutine
    winrt::com_ptr<BaseView> keepInstanceAlive = get_strong();
    co_await BeforeUnloaded();
    CompositionScaleChanged(_compositionScaleChangedEventToken);
    SizeChanged(_sizeChangedEventToken);
    co_return;
  }


  IAsyncAction BaseView::BeforeUnloaded()
  {
    co_return;
  }


  void BaseView::Present()
  {
    DownsampleMultiSampleBackBufferToBackBuffer();
    HandleResult(_dxgiSwapChain->Present1(1, 0, &PresentationParameters));
  }


  ::DirectX::XMFLOAT3 BaseView::RgbFromBrush(SolidColorBrush const& brush)
  {
    Color color = brush.Color();
    return ::DirectX::XMFLOAT3(color.R / 255.0f, color.G / 255.0f, color.B / 255.0f);
  }


  ::DirectX::XMFLOAT4 BaseView::RgbaFromBrush(SolidColorBrush const& brush)
  {
    Color color = brush.Color();
    return ::DirectX::XMFLOAT4(color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f);
  }


  void BaseView::DestroyDeviceResources()
  {
    _d2dContext = nullptr;
    _d3dContext = nullptr;
    _d2dDevice  = nullptr;
    _d3dDevice  = nullptr;

    _multisampleAntiAliasingQuality = 0;
  }


  void BaseView::DestroySizeDependentResources()
  {
    _d3dContext->OMSetRenderTargets(0, nullptr, nullptr);
    _d3dContext->Flush();
    _d2dContext->SetTarget(nullptr);

    _d2dTargetBitmap           = nullptr;
    _d3dDepthStencilView       = nullptr;
    _d3dRenderTargetView       = nullptr;
    _d3dBackBufferMultiSampled = nullptr;
    _d3dBackBuffer             = nullptr;
  }


  void BaseView::CreateD3D11Device()
  {
    winrt::com_ptr<ID3D11Device>        device;
    winrt::com_ptr<ID3D11DeviceContext> context;
    winrt::check_hresult(D3D11CreateDevice(nullptr,                  // Specify null to use the default adapter.
                                           D3D_DRIVER_TYPE_HARDWARE, // Create a device using the hardware graphics driver.
                                           nullptr,                  // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
                                           DeviceCreationFlags,      // Optionally set debug and Direct2D compatibility flags.
                                           FeatureLevels,            // List of feature levels this app can support.
                                           ARRAYSIZE(FeatureLevels), // Size of the list above.
                                           D3D11_SDK_VERSION,        // Always set this to D3D11_SDK_VERSION for Windows Store apps.
                                           device.put(),             // Returns the Direct3D device created.
                                           nullptr,                  // Returns feature level of device created.
                                           context.put())            // Returns the device immediate context.
    );

    _d3dDevice  = device.as<ID3D11Device1>();
    _d3dContext = context.as<ID3D11DeviceContext1>();

    // Ensure that DXGI does not queue more than one frame at a time. This both reduces
    // latency and ensures that the application will only render after each VSync, minimizing
    // power consumption.
    winrt::check_hresult(_d3dDevice.as<IDXGIDevice1>()->SetMaximumFrameLatency(1));
  }


  void BaseView::CreateD2D1Device()
  {
    winrt::check_hresult(_d2dFactory->CreateDevice(_d3dDevice.as<IDXGIDevice>().get(), _d2dDevice.put()));
    winrt::check_hresult(_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, _d2dContext.put()));
    _d2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
  }


  void BaseView::ExtractMultisamplingParameters()
  {
    if(_multisampleAntiAliasingCount > 0)
    {
      UINT formatSupport = 0;
      _d3dDevice->CheckFormatSupport(DxgiFormat, &formatSupport);

      if((formatSupport & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RESOLVE) && (formatSupport & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET))
      {
        _d3dDevice->CheckMultisampleQualityLevels(DxgiFormat, _multisampleAntiAliasingCount, &_multisampleAntiAliasingQuality);
      }
    }
  }


  void BaseView::CalculateRenderTargetSize()
  {
    // The buffers associated with the SwapChainPanel cannot have a width or height of 0. An exception
    // is thrown if that is violated. However, Visual Studio's XAML Designer creates the user control
    // before it knows the final size which means that both ActualWidth and ActualHeight could be 0
    // when the XAML Designer creates the control. This is not the case at runtime because this method
    // is supposed to be invoked after the control is fully loaded, i.e. as reaction to the "Loaded"
    // event. However, to be compatible with the XAML Designer we ensure that the buffers are never of
    // size 0.
    // See the following link for background information regarding this:
    // https://developercommunity.visualstudio.com/t/XAML-Designer-fails-to-render-a-custom-D/875338
    // To debug a XAML control at design time see the following link:
    // https://docs.microsoft.com/en-us/visualstudio/debugger/walkthrough-debugging-at-design-time
    _renderTargetWidth  = std::max(1.0f, static_cast<float>(_compositionScaleX * ActualWidth()));
    _renderTargetHeight = std::max(1.0f, static_cast<float>(_compositionScaleY * ActualHeight()));
  }


  void BaseView::CreateOrResizeSwapChain()
  {
    if(!_dxgiSwapChain)
    {
      CreateSwapChain();
    }

    else
    {
      UINT width  = static_cast<UINT>(_renderTargetWidth);
      UINT height = static_cast<UINT>(_renderTargetHeight);
      HandleResult(_dxgiSwapChain->ResizeBuffers(2, width, height, DxgiFormat, 0));
    }
  }


  void BaseView::CreateSwapChain()
  {
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.Width              = static_cast<UINT>(_renderTargetWidth);
    swapChainDesc.Height             = static_cast<UINT>(_renderTargetHeight);
    swapChainDesc.Format             = DxgiFormat;
    swapChainDesc.Stereo             = false;
    swapChainDesc.SampleDesc.Count   = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount        = 2;
    swapChainDesc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapChainDesc.Flags              = 0;
    swapChainDesc.AlphaMode          = DXGI_ALPHA_MODE_UNSPECIFIED;
    winrt::com_ptr<IDXGISwapChain1> swapChain;
    winrt::check_hresult(_dxgiFactory->CreateSwapChainForComposition(_d3dDevice.get(), &swapChainDesc, nullptr, swapChain.put()));
    winrt::check_hresult(this->try_as<ISwapChainPanelNative>()->SetSwapChain(swapChain.get()));
    _dxgiSwapChain = swapChain.as<IDXGISwapChain2>();
  }


  void BaseView::ScaleSwapChain()
  {
    // Ensure the physical pixel size of the swap chain takes into account both the XAML
    // SwapChainPanel's logical layout size and any cumulative composition scale applied
    // due to zooming, render transforms, or the system's current scaling plateau. For
    // example, if a 100x100 SwapChainPanel has a cumulative 2x scale transform applied,
    // we instead create a 200x200 swap chain to avoid artifacts from scaling it up by
    // 2x, then apply an inverse 1/2x transform to the swap chain to cancel out the 2x
    // transform.
    DXGI_MATRIX_3X2_F inverseScale{};
    inverseScale._11 = 1.0f / _compositionScaleX;
    inverseScale._22 = 1.0f / _compositionScaleY;
    _dxgiSwapChain->SetMatrixTransform(&inverseScale);
  }


  void BaseView::ObtainBackBuffer()
  {
    winrt::check_hresult(_dxgiSwapChain->GetBuffer(0, IID_PPV_ARGS(_d3dBackBuffer.put())));
  }


  void BaseView::CreateRenderTargetView(ID3D11Resource* targetBuffer, const CD3D11_RENDER_TARGET_VIEW_DESC* renderTargetViewDesc)
  {
    winrt::check_hresult(_d3dDevice->CreateRenderTargetView(targetBuffer, renderTargetViewDesc, _d3dRenderTargetView.put()));
  }


  void BaseView::CreateDepthStencilView(UINT count, D3D11_DSV_DIMENSION dsvDimension)
  {
    CD3D11_TEXTURE2D_DESC depthStencilDesc{};
    depthStencilDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.Width              = static_cast<UINT>(_renderTargetWidth);
    depthStencilDesc.Height             = static_cast<UINT>(_renderTargetHeight);
    depthStencilDesc.ArraySize          = 1;
    depthStencilDesc.MipLevels          = 1;
    depthStencilDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.SampleDesc.Count   = count;
    depthStencilDesc.SampleDesc.Quality = 0;
    CD3D11_DEPTH_STENCIL_VIEW_DESC  depthStencilViewDesc(dsvDimension);
    winrt::com_ptr<ID3D11Texture2D> depthStencilBuffer;
    winrt::check_hresult(_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencilBuffer.put()));
    winrt::check_hresult(_d3dDevice->CreateDepthStencilView(depthStencilBuffer.get(), &depthStencilViewDesc, _d3dDepthStencilView.put()));
  }


  void BaseView::CreateMultiSampledBackBuffer()
  {
    D3D11_TEXTURE2D_DESC backBufferMultiSampledDesc{};
    backBufferMultiSampledDesc.Format             = DxgiFormat;
    backBufferMultiSampledDesc.Width              = static_cast<UINT>(_renderTargetWidth);
    backBufferMultiSampledDesc.Height             = static_cast<UINT>(_renderTargetHeight);
    backBufferMultiSampledDesc.BindFlags          = D3D11_BIND_RENDER_TARGET;
    backBufferMultiSampledDesc.MipLevels          = 1;
    backBufferMultiSampledDesc.ArraySize          = 1;
    backBufferMultiSampledDesc.SampleDesc.Count   = _multisampleAntiAliasingCount;
    backBufferMultiSampledDesc.SampleDesc.Quality = 0;
    winrt::check_hresult(_d3dDevice->CreateTexture2D(&backBufferMultiSampledDesc, nullptr, _d3dBackBufferMultiSampled.put()));
  }


  void BaseView::SetD2D1Target(IDXGISurface* targetSurface)
  {
    D2D1_BITMAP_PROPERTIES1 bitmapProperties{};
    bitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
    bitmapProperties.pixelFormat   = D2D1::PixelFormat(DxgiFormat, D2D1_ALPHA_MODE_PREMULTIPLIED);
    bitmapProperties.dpiX          = _compositionScaleX;
    bitmapProperties.dpiY          = _compositionScaleY;
    winrt::check_hresult(_d2dContext->CreateBitmapFromDxgiSurface(targetSurface, &bitmapProperties, _d2dTargetBitmap.put()));
    _d2dContext->SetDpi(bitmapProperties.dpiX, bitmapProperties.dpiY);
    _d2dContext->SetTarget(_d2dTargetBitmap.get());
  }


  void BaseView::DownsampleMultiSampleBackBufferToBackBuffer()
  {
    if(_multisampleAntiAliasingQuality > 0)
    {
      const UINT index = D3D11CalcSubresource(0, 0, 1);
      _d3dContext->ResolveSubresource(_d3dBackBuffer.get(), index, _d3dBackBufferMultiSampled.get(), index, DxgiFormat);
    }
  }


  void BaseView::HandleResult(HRESULT result)
  {
    if(result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET)
    {
      OnDeviceLost();
    }

    else
    {
      winrt::check_hresult(result);
    }
  }
}
