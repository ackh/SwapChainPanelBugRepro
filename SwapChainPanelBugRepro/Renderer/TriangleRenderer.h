#pragma once

#include <DirectXMath.h>
#include <functional>
#include "../Vertex/Vertex.h"


namespace SwapChainPanelBugRepro
{
  class TriangleRenderer
  {
  public:
    void SetScreenSize(float width, float height);
    void RenderFrame();
    void SetDevice(ID3D11Device1* d3dDevice);
    void SetContext(ID3D11DeviceContext1* d3dContext);
    void SetRenderTargetView(ID3D11RenderTargetView* renderTargetView);
    void SetDepthStencilView(ID3D11DepthStencilView* depthStencilView);
    void CreateDeviceResources();

  private:
    static constexpr UINT OFFSET        = 0;
    static constexpr UINT VERTEX_SIZE   = sizeof(Vertex);
    static constexpr int  VERTEX_COUNT  = 3;
    static constexpr int  VERTICES_SIZE = VERTEX_COUNT * VERTEX_SIZE;
    static constexpr int  INDICES_SIZE  = VERTEX_COUNT * sizeof(int);

    static constexpr Vertex VERTICES[VERTEX_COUNT] =
    {
      { {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
      { {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
      { { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    };

    static constexpr int INDICES[VERTEX_COUNT] =
    {
      0, 1, 2,
    };

    static constexpr ::DirectX::XMFLOAT4 BACKGROUND_COLOR = { 0.0f, 0.0f, 0.0f, 0.0f };

    ID3D11Device1*          _d3dDevice;
    ID3D11DeviceContext1*   _d3dContext;
    ID3D11RenderTargetView* _d3dRenderTargetView;
    ID3D11DepthStencilView* _d3dDepthStencilView;

    CD3D11_VIEWPORT _viewPort{};

    winrt::com_ptr<ID3D11InputLayout>  _inputLayout;
    winrt::com_ptr<ID3D11VertexShader> _vertexShader;
    winrt::com_ptr<ID3D11PixelShader>  _pixelShader;
    winrt::com_ptr<ID3D11Buffer>       _vertexBuffer;
    winrt::com_ptr<ID3D11Buffer>       _indexBuffer;

    void CreateVertexShader();
    void CreatePixelShader();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void ConfigurePipeline();
  };
}
