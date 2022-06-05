#include "CoreCommon.h"
#include "PixelBuffer.h"

class SwapChainBuffer : public PixelBuffer
{
public:
  SwapChainBuffer() = default;
  SwapChainBuffer(IDXGISwapChain1 *pSwapChain, uint32_t bufferIndex, D3D12_RESOURCE_STATES resourseState, const std::wstring &name);
};