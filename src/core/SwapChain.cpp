#include "SwapChain.h"

void SwapChain::create()
{
  createSwapChain();
  createRtvHeap();
  createSwapChainBuffers();
}

void SwapChain::createSwapChain()
{
  ComPtr<IDXGIFactory7> dxgiFactory;

  ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

  DXGI_SWAP_CHAIN_DESC1 sd{};
  sd.Width = g_displayWidth;
  sd.Height = g_displayHeight;
  sd.Format = m_BackBufferFormat;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
  sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  sd.Scaling = DXGI_SCALING_NONE;

  DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc{};
  fsSwapChainDesc.RefreshRate.Numerator = 60;
  fsSwapChainDesc.RefreshRate.Denominator = 1;
  fsSwapChainDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  fsSwapChainDesc.Windowed = true;

  IDXGISwapChain1 *tmpSwapChain = nullptr;

  dxgiFactory->CreateSwapChainForHwnd(D3DContext::GetCommandQueue(), g_hwnd, &sd, &fsSwapChainDesc, nullptr, &tmpSwapChain);
  tmpSwapChain->QueryInterface(__uuidof(IDXGISwapChain3), (void **)(m_swapChain.GetAddressOf()));
}

void SwapChain::createRtvHeap()
{
  D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
  rtvHeapDesc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT;
  rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  rtvHeapDesc.NodeMask = 0;

  ThrowIfFailed(D3DContext::GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_rtvHeap.GetAddressOf())));
}

void SwapChain::createSwapChainBuffers()
{
  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

  m_rtvDescriptorSize = D3DContext::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  for (uint32_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
  {
    m_buffer[i] = SwapChainBuffer(m_swapChain.Get(), i, D3D12_RESOURCE_STATE_PRESENT, L"Swap Chain Buffer");
    D3DContext::GetDevice()->CreateRenderTargetView(m_buffer[i].getResource(), nullptr, rtvHandle);
    rtvHandle.Offset(m_rtvDescriptorSize);
  }
}

void SwapChain::present()
{
  m_swapChain->Present(0, 0);
}

void SwapChain::transitionBuffer(D3D12_RESOURCE_STATES newState, uint32_t bufferIndex)
{
  auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
      m_buffer[bufferIndex].getResource(), m_buffer[bufferIndex].getCurrentStage(),
      newState);

  m_buffer[bufferIndex].setCurrentStage(newState);

  D3DContext::GetCommandList()->ResourceBarrier(1, &barrier);
}
