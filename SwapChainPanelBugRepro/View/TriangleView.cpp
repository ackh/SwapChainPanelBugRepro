#include "pch.h"
#include "TriangleView.h"
#include "TriangleView.g.cpp"


namespace winrt::SwapChainPanelBugRepro::implementation
{
  using namespace winrt::Microsoft::UI::Dispatching;
  using namespace winrt::Microsoft::UI::Input;
  using namespace winrt::Windows::Foundation;


  void TriangleView::CreateDeviceResources()
  {
    _renderer->SetContext(nullptr);
    _renderer->SetDevice(nullptr);
    BaseView::CreateDeviceResources();
    _renderer->SetContext(_d3dContext.get());
    _renderer->SetDevice(_d3dDevice.get());
    _renderer->CreateDeviceResources();
  }


  void TriangleView::CreateSizeDependentResources()
  {
    _renderer->SetRenderTargetView(nullptr);
    _renderer->SetDepthStencilView(nullptr);
    BaseView::CreateSizeDependentResources();
    _renderer->SetRenderTargetView(_d3dRenderTargetView.get());
    _renderer->SetDepthStencilView(_d3dDepthStencilView.get());
    _renderer->SetScreenSize(_renderTargetWidth, _renderTargetHeight);
  }


  IAsyncAction TriangleView::AfterLoaded()
  {
    _renderer = std::make_unique<::SwapChainPanelBugRepro::TriangleRenderer>();
    CreateDeviceIndependentResources();
    CreateDeviceResources();
    CreateSizeDependentResources();
    StartRenderingLoop();
    StartInputProcessingLoop();
    co_return;
  }


  IAsyncAction TriangleView::BeforeUnloaded()
  {
    _isRenderLoopRunning = false;
    co_await _renderLoopController.ShutdownQueueAsync();
    co_await _inputProcessingLoopController.ShutdownQueueAsync();
    co_return;
  }


  void TriangleView::StartRenderingLoop()
  {
    if(!_isRenderLoopRunning)
    {
      _renderLoopController = DispatcherQueueController::CreateOnDedicatedThread();
      _isRenderLoopRunning  = _renderLoopController.DispatcherQueue().TryEnqueue([this]()
      {
        while(_isRenderLoopRunning)
        {
          Render();
          _dxgiOutput->WaitForVBlank();
        }
      });
    }
  }


  void TriangleView::StartInputProcessingLoop()
  {
    if(!_inputProcessingLoopController)
    {
      _inputProcessingLoopController = DispatcherQueueController::CreateOnDedicatedThread();
      _inputProcessingLoopController.DispatcherQueue().TryEnqueue([this]()
      {
        InputPointerSource inputPointerSource = CreateCoreIndependentInputSource(_pointerSourceDevices);
        inputPointerSource.PointerPressed({ this, &TriangleView::OnPointerPressed });
      });
    }
  }


  void TriangleView::Render()
  {
    _renderer->RenderFrame();
    Present();
  }


  void TriangleView::OnPointerPressed(IInspectable const&, PointerEventArgs)
  {
    // Normally, in here we would process mouse, pen, etc. input and execute code accordingly.
    // However, the mere presence of the input processing loop leads to an exception when
    // cleaning up the SwapChainPanel.
  }
}
