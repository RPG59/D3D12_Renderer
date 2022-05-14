#pragma once

#include <wrl/client.h>
#include <d3d12.h>

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

protected:
  ComPtr<ID3D12Resource1> m_pResoucrce;
  D3D12_GPU_VIRTUAL_ADDRESS m_gpuVirtualAddress = NULL;
  D3D12_RESOURCE_STATES m_currentState = D3D12_RESOURCE_STATE_COMMON;
};