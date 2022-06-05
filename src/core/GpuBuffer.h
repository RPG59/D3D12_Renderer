#pragma once

#include "CoreCommon.h"
#include "GpuResource.h"
#include "UploadBuffer.h"

class GpuBuffer : public GpuResource
{
public:
  void create(const std::wstring &name, uint32_t numElements, uint32_t elementSize, const UploadBuffer &uploadBuffer);

  uint32_t getElementCount() const
  {
    return m_elementCount;
  }

  uint32_t getElementSize() const
  {
    return m_elementSize;
  }

  size_t getBufferSize() const
  {
    return m_bufferSize;
  }

  D3D12_VERTEX_BUFFER_VIEW getVertexBufferView(size_t offset, uint32_t size, uint32_t stride) const
  {
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    vertexBufferView.BufferLocation = m_gpuVirtualAddress + offset;
    vertexBufferView.SizeInBytes = size;
    vertexBufferView.StrideInBytes = stride;

    return vertexBufferView;
  }

  D3D12_INDEX_BUFFER_VIEW getIndexBufferView(size_t offset, uint32_t size, bool b32BitFormat = false) const
  {
    D3D12_INDEX_BUFFER_VIEW indexBufferView;
    indexBufferView.BufferLocation = m_gpuVirtualAddress + offset;
    indexBufferView.SizeInBytes = size;
    indexBufferView.Format = b32BitFormat ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
  }

protected:
  uint32_t m_elementSize;
  uint32_t m_elementCount;
  size_t m_bufferSize;
};