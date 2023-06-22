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
    winrt::Microsoft::UI::Dispatching::DispatcherQueueController _inputProcessingLoopController{ nullptr };

    static constexpr auto _pointerSourceDevices =
      winrt::Microsoft::UI::Input::InputPointerSourceDeviceKinds::Mouse |
      winrt::Microsoft::UI::Input::InputPointerSourceDeviceKinds::Touch |
      winrt::Microsoft::UI::Input::InputPointerSourceDeviceKinds::Pen;

    bool _isRenderLoopRunning = false;

    void CreateDeviceResources() override;
    void CreateSizeDependentResources() override;

    winrt::Windows::Foundation::IAsyncAction AfterLoaded() override;
    winrt::Windows::Foundation::IAsyncAction BeforeUnloaded() override;

    void StartRenderingLoop();
    void StartInputProcessingLoop();
    void Render();

    void OnPointerPressed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Input::PointerEventArgs e);
  };
}


namespace winrt::SwapChainPanelBugRepro::factory_implementation
{
  struct TriangleView : TriangleViewT<TriangleView, implementation::TriangleView>
  {
  };
}
