#pragma once

#include "CoreCommon.h"
#include "PixelBuffer.h"
#include "D3DContext.h"
#include "SwapChainBuffer.h"

class SwapChain
{
public:
  void create();
  void present();
  void transitionBuffer(D3D12_RESOURCE_STATES newState, uint32_t bufferIndex);

  const D3D12_CPU_DESCRIPTOR_HANDLE &getRtv(uint32_t currentBackBufferIndex) const
  {
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), currentBackBufferIndex, m_rtvDescriptorSize);
  }

  IDXGISwapChain1 *_getSwapChain()
  {
    return m_swapChain.Get();
  }

  uint32_t getCurrentBackBufferIndex()
  {
    return m_swapChain->GetCurrentBackBufferIndex();
  }

private:
  SwapChainBuffer m_buffer[SWAP_CHAIN_BUFFER_COUNT];
  ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
  ComPtr<IDXGISwapChain3> m_swapChain;
  DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
  uint32_t m_rtvDescriptorSize = 0;

  void createSwapChain();
  void createSwapChainBuffers();
  void createRtvHeap();
};