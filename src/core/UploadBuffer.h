#pragma once

#include <string>

#include "GpuResource.h"

class UploadBuffer : public GpuResource
{
public:
  void create(const std::wstring &name, size_t size);
  void *map(void);
  void unmap(size_t begin = 0, size_t end = -1);

  size_t getBufferSize() const
  {
    return m_bufferSize;
  }

private:
  size_t m_bufferSize;
};
