#include "GpuBuffer.h"
#include "D3DContext.h"

void GpuBuffer::create(const std::wstring &name, uint32_t numElements, uint32_t elementSize, const UploadBuffer &uploadBuffer)
{
  m_elementCount = numElements;
  m_elementSize = elementSize;
  m_bufferSize = elementSize * numElements;

  auto bufferProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize);
  auto state = D3D12_RESOURCE_STATE_COPY_DEST;

  ThrowIfFailed(D3DContext::GetDevice()->CreateCommittedResource(
      &bufferProps,
      D3D12_HEAP_FLAG_NONE,
      &bufferDesc,
      state,
      nullptr,
      IID_PPV_ARGS(m_pResoucrce.GetAddressOf())));

  m_gpuVirtualAddress = m_pResoucrce->GetGPUVirtualAddress();

  // auto copyBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_pResoucrce.Get(), m_currentState, state);

  // D3DContext::GetCommandList()->ResourceBarrier(1, &copyBarrier);
  D3DContext::GetCommandList()
      ->CopyBufferRegion(m_pResoucrce.Get(), 0, uploadBuffer.getResource(), 0, m_bufferSize);

  // auto readBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_pResoucrce.Get(), state, D3D12_RESOURCE_STATE_GENERIC_READ);
  auto readBarrier = CD3DX12_RESOURCE_BARRIER::Transition(m_pResoucrce.Get(), state, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

  D3DContext::GetCommandList()->ResourceBarrier(1, &readBarrier);

  m_currentState = D3D12_RESOURCE_STATE_GENERIC_READ;

#ifndef RELEASE
  m_pResoucrce->SetName(name.c_str());
#endif
}
