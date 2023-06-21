#include "pch.h"
#include "App.h"
#include "MainWindow.h"


namespace winrt::SwapChainPanelBugRepro::implementation
{
  using namespace winrt::Microsoft::UI::Xaml;


  App::App()
  {
    InitializeComponent();

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
    UnhandledException([](IInspectable const&, UnhandledExceptionEventArgs const& e) {
      if(IsDebuggerPresent())
      {
        auto errorMessage = e.Message();
        __debugbreak();
      }
    });
#endif
  }


  void App::OnLaunched(LaunchActivatedEventArgs const&)
  {
    window = winrt::make<MainWindow>();
    window.Activate();
  }
}
