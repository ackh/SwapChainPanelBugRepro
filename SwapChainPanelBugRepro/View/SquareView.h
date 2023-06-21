#pragma once

#include "BaseView.h"
#include "SquareView.g.h"
#include "../Renderer/SquareRenderer.h"


namespace winrt::SwapChainPanelBugRepro::implementation
{
  struct SquareView : SquareViewT<SquareView, BaseView>
  {
  private:
    std::unique_ptr<::SwapChainPanelBugRepro::SquareRenderer> _renderer;

    winrt::Microsoft::UI::Dispatching::DispatcherQueueController _renderLoopController{ nullptr };

    bool _isRenderLoopRunning = false;

    void CreateDeviceResources() override;
    void CreateSizeDependentResources() override;

    winrt::Windows::Foundation::IAsyncAction AfterLoaded() override;
    winrt::Windows::Foundation::IAsyncAction BeforeUnloaded() override;

    void StartRenderingLoop();
    void Render();
  };
}


namespace winrt::SwapChainPanelBugRepro::factory_implementation
{
  struct SquareView : SquareViewT<SquareView, implementation::SquareView>
  {
  };
}
