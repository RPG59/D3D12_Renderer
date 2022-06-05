#pragma once
#include "GpuResource.h"

class PixelBuffer : public GpuResource
{
protected:
  uint32_t m_width;
  uint32_t m_height;
  DXGI_FORMAT m_format;
};