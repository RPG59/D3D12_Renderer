#pragma once

#include "SwapChain.h"
#include "DepthBuffer.h"

class Display
{
public:
  void init();
  void present();
  void prepearePresent();

  static Display *getInstance();

  SwapChain *getSwapchain()
  {
    return m_swapChain;
  }

  uint32_t getCurrentBackBufferIndex()
  {
    return m_swapChain->getCurrentBackBufferIndex();
  }

private:
  static Display *instance;
  uint64_t m_frameIndex = 0;
  SwapChain *m_swapChain = nullptr;
  DepthBuffer *m_depthBuffer = nullptr;
};
