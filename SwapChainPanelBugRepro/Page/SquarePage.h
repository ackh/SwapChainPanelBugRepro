#pragma once

#include "SquarePage.g.h"


namespace winrt::SwapChainPanelBugRepro::implementation
{
  struct SquarePage : SquarePageT<SquarePage>
  {
  };
}


namespace winrt::SwapChainPanelBugRepro::factory_implementation
{
  struct SquarePage : SquarePageT<SquarePage, implementation::SquarePage>
  {
  };
}
