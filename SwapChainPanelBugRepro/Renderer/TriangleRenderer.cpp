#include "pch.h"
#include "TriangleRenderer.h"
#include "../Shader/Compiled/VertexShaderCode.h"
#include "../Shader/Compiled/PixelShaderCode.h"

using namespace SwapChainPanelBugRepro;


void TriangleRenderer::SetScreenSize(float width, float height)
{
  _viewPort = CD3D11_VIEWPORT(0.0f, 0.0f, width, height);
  _d3dContext->RSSetViewports(1, &_viewPort);
}


void TriangleRenderer::RenderFrame()
{
  _d3dContext->OMSetRenderTargets(1, &_d3dRenderTargetView, _d3dDepthStencilView);
  _d3dContext->ClearRenderTargetView(_d3dRenderTargetView, reinterpret_cast<float const*>(&BACKGROUND_COLOR));
  _d3dContext->ClearDepthStencilView(_d3dDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
  _d3dContext->DrawIndexed(VERTEX_COUNT, 0, 0);
}


void TriangleRenderer::SetDevice(ID3D11Device1* d3dDevice)
{
  _d3dDevice = d3dDevice;
}


void TriangleRenderer::SetContext(ID3D11DeviceContext1* d3dContext)
{
  _d3dContext = d3dContext;
}


void TriangleRenderer::SetRenderTargetView(ID3D11RenderTargetView* renderTargetView)
{
  _d3dRenderTargetView = renderTargetView;
}


void TriangleRenderer::SetDepthStencilView(ID3D11DepthStencilView* depthStencilView)
{
  _d3dDepthStencilView = depthStencilView;
}


void TriangleRenderer::CreateDeviceResources()
{
  CreateVertexShader();
  CreatePixelShader();
  CreateVertexBuffer();
  CreateIndexBuffer();
  ConfigurePipeline();
}


void TriangleRenderer::CreateVertexShader()
{
  static constexpr D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
  {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, coordinates), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR"   , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, color)      , D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };

  auto   shaderCode = reinterpret_cast<std::byte const*>(&_vertexShaderCode);
  SIZE_T shaderSize = ARRAYSIZE(_vertexShaderCode);

  winrt::check_hresult(_d3dDevice->CreateVertexShader(shaderCode, shaderSize, nullptr, _vertexShader.put()));
  winrt::check_hresult(_d3dDevice->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), shaderCode, shaderSize, _inputLayout.put()));
}


void TriangleRenderer::CreatePixelShader()
{
  auto   shaderCode = reinterpret_cast<std::byte const*>(&_pixelShaderCode);
  SIZE_T shaderSize = ARRAYSIZE(_pixelShaderCode);

  winrt::check_hresult(_d3dDevice->CreatePixelShader(shaderCode, shaderSize, nullptr, _pixelShader.put()));
  _d3dContext->PSSetShader(_pixelShader.get(), nullptr, 0);
}


void TriangleRenderer::CreateVertexBuffer()
{
  D3D11_SUBRESOURCE_DATA vertexBufferData{};
  vertexBufferData.pSysMem = VERTICES;

  D3D11_BUFFER_DESC vertexBufferDesc{};
  vertexBufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
  vertexBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
  vertexBufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
  vertexBufferDesc.ByteWidth           = VERTICES_SIZE;
  vertexBufferDesc.StructureByteStride = VERTEX_SIZE;

  winrt::check_hresult(_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, _vertexBuffer.put()));
}


void TriangleRenderer::CreateIndexBuffer()
{
  D3D11_SUBRESOURCE_DATA indexBufferData{};
  indexBufferData.pSysMem = INDICES;

  D3D11_BUFFER_DESC indexBufferDesc{};
  indexBufferDesc.Usage     = D3D11_USAGE_IMMUTABLE;
  indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  indexBufferDesc.ByteWidth = INDICES_SIZE;

  winrt::check_hresult(_d3dDevice->CreateBuffer(&indexBufferDesc, &indexBufferData, _indexBuffer.put()));
}


void TriangleRenderer::ConfigurePipeline()
{
  ID3D11Buffer* cubeVertexBuffer = _vertexBuffer.get();
  _d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  _d3dContext->VSSetShader(_vertexShader.get(), nullptr, 0); _d3dContext->IASetInputLayout(_inputLayout.get());
  _d3dContext->IASetVertexBuffers(0, 1, &cubeVertexBuffer, &VERTEX_SIZE, &OFFSET);
  _d3dContext->IASetIndexBuffer(_indexBuffer.get(), DXGI_FORMAT_R32_UINT, 0);
}
