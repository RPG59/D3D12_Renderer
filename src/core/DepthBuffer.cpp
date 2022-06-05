#include "DepthBuffer.h"
#include "D3DContext.h"

DepthBuffer::DepthBuffer()
{
  D3D12_DESCRIPTOR_HEAP_DESC rhd_dsv{};
  rhd_dsv.NumDescriptors = 1;
  rhd_dsv.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
  rhd_dsv.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  rhd_dsv.NodeMask = 0;

  ThrowIfFailed(D3DContext::GetDevice()->CreateDescriptorHeap(&rhd_dsv, IID_PPV_ARGS(m_dsvHeap.GetAddressOf())));

#ifndef RELEASE
  m_dsvHeap->SetName(L"DSV Heap");
#endif

  D3D12_DEPTH_STENCIL_VIEW_DESC dsvd{};
  dsvd.Format = DXGI_FORMAT_D32_FLOAT;
  dsvd.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
  dsvd.Flags = D3D12_DSV_FLAG_NONE;

  D3D12_CLEAR_VALUE depthOptimizedCliarValue{};
  depthOptimizedCliarValue.Format = DXGI_FORMAT_D32_FLOAT;
  depthOptimizedCliarValue.DepthStencil.Depth = 1.0;
  depthOptimizedCliarValue.DepthStencil.Stencil = 0;

  auto depthStencilBufferProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  auto depthStencilBufferDesc =
      CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, g_displayWidth, g_displayHeight, 1, 0, 1, 0,
                                   D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

  D3DContext::GetDevice()->CreateCommittedResource(
      &depthStencilBufferProps,
      D3D12_HEAP_FLAG_NONE,
      &depthStencilBufferDesc,
      D3D12_RESOURCE_STATE_DEPTH_WRITE,
      &depthOptimizedCliarValue,
      IID_PPV_ARGS(m_pResoucrce.GetAddressOf()));

#ifndef RELEASE
  m_pResoucrce->SetName(L"Depth Buffer");
#endif

  D3DContext::GetDevice()->CreateDepthStencilView(m_pResoucrce.Get(), &dsvd,
                                                  m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

  CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

  // FIXME!
  m_dsvHandle = dsvHandle;
}

void DepthBuffer::clear()
{
  D3DContext::GetCommandList()->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1., 0,
                                                      0, nullptr);
}