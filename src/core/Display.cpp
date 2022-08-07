#pragma once

#include "Display.h"
#include "D3DContext.h"

void Display::init()
{
  m_swapChain = new SwapChain();
  m_swapChain->create();

  m_depthBuffer = new DepthBuffer();
}
void Display::prepearePresent()
{
  auto context = D3DContext::getContext();
  auto currentBackBufferIndex = getCurrentBackBufferIndex();

  m_swapChain->transitionBuffer(D3D12_RESOURCE_STATE_RENDER_TARGET, currentBackBufferIndex);

  D3DContext::GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  m_depthBuffer->clear();

  auto rtvHandle = m_swapChain->getRtv(currentBackBufferIndex);
  const float color[4] = {.3, .4, .1, .0};

  D3DContext::GetCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, &(m_depthBuffer->getDsv()));
  D3DContext::GetCommandList()->ClearRenderTargetView(rtvHandle, color, 0, nullptr);

  context->SetGraphicsRootSignature();
  context->SetViewport();
  context->SetScissor();
  context->SetBuffers();

  m_swapChain->transitionBuffer(D3D12_RESOURCE_STATE_PRESENT, currentBackBufferIndex);
}

void Display::present()
{
  m_swapChain->present();
}

Display *Display::getInstance()
{
  if (Display::instance == nullptr)
  {
    Display::instance = new Display();
  }

  return Display::instance;
}

Display *Display::instance = nullptr;
