#include "UploadBuffer.h"
#include "D3DContext.h"

void UploadBuffer::create(const std::wstring &name, size_t size)
{
  auto uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  auto uploadHeapDesc = CD3DX12_RESOURCE_DESC::Buffer(size);

  ThrowIfFailed(D3DContext::GetDevice()->CreateCommittedResource(
      &uploadHeapProps,
      D3D12_HEAP_FLAG_NONE,
      &uploadHeapDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(m_pResoucrce.GetAddressOf())));

#ifndef RELEASE
  m_pResoucrce->SetName(name.c_str());
#endif

  m_gpuVirtualAddress = m_pResoucrce->GetGPUVirtualAddress();
}

void *UploadBuffer::map(void)
{
  void *ptr;
  auto range = CD3DX12_RANGE(0, m_bufferSize);

  ThrowIfFailed(m_pResoucrce->Map(0, &range, &ptr));

  return ptr;
}

void UploadBuffer::unmap(size_t begin, size_t end)
{
  auto range = CD3DX12_RANGE(begin, std::min(end, m_bufferSize));
  m_pResoucrce->Unmap(0, &range);
}