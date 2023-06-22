#include "pch.h"
#include "SquareView.h"
#include "SquareView.g.cpp"


namespace winrt::SwapChainPanelBugRepro::implementation
{
  using namespace winrt::Microsoft::UI::Dispatching;
  using namespace winrt::Microsoft::UI::Input;
  using namespace winrt::Windows::Foundation;


  void SquareView::CreateDeviceResources()
  {
    _renderer->SetContext(nullptr);
    _renderer->SetDevice(nullptr);
    BaseView::CreateDeviceResources();
    _renderer->SetContext(_d3dContext.get());
    _renderer->SetDevice(_d3dDevice.get());
    _renderer->CreateDeviceResources();
  }


  void SquareView::CreateSizeDependentResources()
  {
    _renderer->SetRenderTargetView(nullptr);
    _renderer->SetDepthStencilView(nullptr);
    BaseView::CreateSizeDependentResources();
    _renderer->SetRenderTargetView(_d3dRenderTargetView.get());
    _renderer->SetDepthStencilView(_d3dDepthStencilView.get());
    _renderer->SetScreenSize(_renderTargetWidth, _renderTargetHeight);
  }


  IAsyncAction SquareView::AfterLoaded()
  {
    _renderer = std::make_unique<::SwapChainPanelBugRepro::SquareRenderer>();
    CreateDeviceIndependentResources();
    CreateDeviceResources();
    CreateSizeDependentResources();
    StartRenderingLoop();
    StartInputProcessingLoop();
    co_return;
  }


  IAsyncAction SquareView::BeforeUnloaded()
  {
    _isRenderLoopRunning = false;
    co_await _renderLoopController.ShutdownQueueAsync();
    co_await _inputProcessingLoopController.ShutdownQueueAsync();
    co_return;
  }


  void SquareView::StartRenderingLoop()
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


  void SquareView::StartInputProcessingLoop()
  {
    if(!_inputProcessingLoopController)
    {
      _inputProcessingLoopController = DispatcherQueueController::CreateOnDedicatedThread();
      _inputProcessingLoopController.DispatcherQueue().TryEnqueue([this]()
      {
        InputPointerSource inputPointerSource = CreateCoreIndependentInputSource(_pointerSourceDevices);
        inputPointerSource.PointerPressed({ this, &SquareView::OnPointerPressed });
      });
    }
  }


  void SquareView::Render()
  {
    _renderer->RenderFrame();
    Present();
  }


  void SquareView::OnPointerPressed(IInspectable const&, PointerEventArgs)
  {
    // Normally, in here we would process mouse, pen, etc. input and execute code accordingly.
    // However, the mere presence of the input processing loop leads to an exception when
    // cleaning up the SwapChainPanel.
  }
}
