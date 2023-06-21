#pragma once

#include <functional>
#include "MainWindow.g.h"


namespace winrt::SwapChainPanelBugRepro::implementation
{
  struct MainWindow : MainWindowT<MainWindow>
  {
  public:
    void OnLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args) const;
    void OnSelectionChanged(winrt::Microsoft::UI::Xaml::Controls::NavigationView sender, winrt::Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs args) const;

  private:
    static const std::map<std::string, winrt::Windows::UI::Xaml::Interop::TypeName, std::less<>> _tagValueToPageTypeMap;
  };
}


namespace winrt::SwapChainPanelBugRepro::factory_implementation
{
  struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
  {
  };
}
