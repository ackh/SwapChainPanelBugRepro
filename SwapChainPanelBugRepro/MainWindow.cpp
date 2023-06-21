#include "pch.h"
#include "MainWindow.h"
#include "MainWindow.g.cpp"


namespace winrt::SwapChainPanelBugRepro::implementation
{
  using namespace winrt;
  using namespace winrt::Microsoft::UI::Xaml;
  using namespace winrt::Microsoft::UI::Xaml::Controls;
  using namespace winrt::Windows::UI::Xaml::Interop;


  void MainWindow::OnLoaded(IInspectable const& sender, RoutedEventArgs const&) const
  {
    NavigationView navigationView = sender.try_as<NavigationView>();
    navigationView.SelectedItem(navigationView.MenuItems().GetAt(0));
  }


  void MainWindow::OnSelectionChanged(NavigationView sender, NavigationViewSelectionChangedEventArgs args) const
  {
    std::string selectedItemTagValue = to_string(unbox_value<hstring>(args.SelectedItem().try_as<NavigationViewItem>().Tag()));
    sender.Content().try_as<Frame>().Navigate(_tagValueToPageTypeMap.at(selectedItemTagValue));
  }


  const std::map<std::string, TypeName, std::less<>> MainWindow::_tagValueToPageTypeMap =
  {
    { "TrianglePage", xaml_typename<TrianglePage>() },
    { "SquarePage"  , xaml_typename<SquarePage>()   },
  };
}
