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

protected:
  uint32_t m_elementSize;
  uint32_t m_elementCount;
  size_t m_bufferSize;
};