#pragma once

#include "BaseView.h"
#include "TriangleView.g.h"
#include "../Renderer/TriangleRenderer.h"


namespace winrt::SwapChainPanelBugRepro::implementation
{
  struct TriangleView : TriangleViewT<TriangleView, BaseView>
  {
  private:
    std::unique_ptr<::SwapChainPanelBugRepro::TriangleRenderer> _renderer;

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
  struct TriangleView : TriangleViewT<TriangleView, implementation::TriangleView>
  {
  };
}
