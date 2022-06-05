#include "SwapChainBuffer.h"

SwapChainBuffer::SwapChainBuffer(IDXGISwapChain1 *pSwapChain, uint32_t bufferIndex, D3D12_RESOURCE_STATES resourseState, const std::wstring &name)
{
  pSwapChain->GetBuffer(bufferIndex, IID_PPV_ARGS(m_pResoucrce.GetAddressOf()));
  m_currentState = resourseState;

#ifndef RELEASE
  m_pResoucrce->SetName(name.c_str());
#endif

  auto resourceDesc = m_pResoucrce->GetDesc();

  m_width = resourceDesc.Width;
  m_height = resourceDesc.Height;
  m_format = resourceDesc.Format;
}
