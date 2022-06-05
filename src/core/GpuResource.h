#pragma once

#include "CoreCommon.h"

using Microsoft::WRL::ComPtr;

class GpuResource
{
public:
  ~GpuResource()
  {
    m_pResoucrce = nullptr;
  }

  D3D12_GPU_VIRTUAL_ADDRESS getGpuVirtualAddress() const
  {
    return m_gpuVirtualAddress;
  }

  ID3D12Resource *getResource() const
  {
    return m_pResoucrce.Get();
  }

  const D3D12_RESOURCE_STATES getUseageState() const
  {
    return m_usageState;
  }

  const D3D12_RESOURCE_STATES getCurrentStage() const
  {
    return m_currentState;
  }

  void setCurrentStage(const D3D12_RESOURCE_STATES state)
  {
    m_currentState = state;
  }

protected:
  ComPtr<ID3D12Resource1> m_pResoucrce;
  D3D12_GPU_VIRTUAL_ADDRESS m_gpuVirtualAddress = NULL;
  D3D12_RESOURCE_STATES m_currentState = D3D12_RESOURCE_STATE_COMMON;
  D3D12_RESOURCE_STATES m_usageState = D3D12_RESOURCE_STATE_COMMON;
};