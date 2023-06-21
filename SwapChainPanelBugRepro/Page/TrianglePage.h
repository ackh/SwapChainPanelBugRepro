#pragma once

#include "TrianglePage.g.h"


namespace winrt::SwapChainPanelBugRepro::implementation
{
  struct TrianglePage : TrianglePageT<TrianglePage>
  {
  };
}


namespace winrt::SwapChainPanelBugRepro::factory_implementation
{
  struct TrianglePage : TrianglePageT<TrianglePage, implementation::TrianglePage>
  {
  };
}
