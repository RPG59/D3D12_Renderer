#include "PixelBuffer.h"

class DepthBuffer : public PixelBuffer
{
public:
  DepthBuffer();

  void clear();

  const D3D12_CPU_DESCRIPTOR_HANDLE &getDsv() const
  {
    return m_dsvHandle;
  }

private:
  ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
  D3D12_CPU_DESCRIPTOR_HANDLE m_dsvHandle;
};