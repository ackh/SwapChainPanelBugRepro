﻿#pragma once

#include "App.xaml.g.h"


namespace winrt::SwapChainPanelBugRepro::implementation
{
  struct App : AppT<App>
  {
  public:
    App();
    void OnLaunched(winrt::Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);

  private:
    winrt::Microsoft::UI::Xaml::Window window{ nullptr };
  };
}
