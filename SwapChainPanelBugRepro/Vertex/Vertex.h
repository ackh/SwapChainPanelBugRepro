#pragma once

#include <DirectXMath.h>


namespace SwapChainPanelBugRepro
{
  struct Vertex
  {
    constexpr Vertex(::DirectX::XMFLOAT3 coordinates, ::DirectX::XMFLOAT3 color) : coordinates(coordinates), color(color)
    {
    }

    ::DirectX::XMFLOAT3 coordinates;
    ::DirectX::XMFLOAT3 color;
  };
}
