//
// Created by RPG59 on 9/27/2020.
//

#include <comdef.h>
#include "Scene.h"
#include "../Util.h"

void Scene::Draw() {
    //ThrowIfFailed(D3DContext::GetCommandAllocator()->Reset());
    //ThrowIfFailed(D3DContext::GetCommandList()->Reset(D3DContext::GetCommandAllocator().Get(), nullptr));

    //D3DContext::GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
    //    D3DContext::getCurrBackBuffer().Get(), 
    //    D3D12_RESOURCE_STATE_PRESENT, 
    //    D3D12_RESOURCE_STATE_RENDER_TARGET
    //    ));

    //CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(D3DContext::GetRtvHeap()->GetCPUDescriptorHandleForHeapStart());

    //D3DContext::GetCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    //const float color[4] = {.3, .4, .1, .0};
    //D3DContext::GetCommandList()->ClearRenderTargetView(rtvHandle, color, 0, nullptr);

    //D3DContext::GetCommandList()->SetGraphicsRootSignature(D3DContext::GetRootSignature().Get());
    //D3DContext::GetCommandList()->RSSetViewports(1, &D3DContext::GetViewport());
    //D3DContext::GetCommandList()->RSSetScissorRects(1, &D3DContext::GetScissor());
    //D3DContext::GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    //D3DContext::GetCommandList()->IASetVertexBuffers(0, 1, &D3DContext::GetVertexBufferView());
    //D3DContext::GetCommandList()->DrawInstanced(3, 1, 0, 0);


    //D3DContext::GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
    //    D3DContext::getCurrBackBuffer().Get(),
    //    D3D12_RESOURCE_STATE_RENDER_TARGET,
    //    D3D12_RESOURCE_STATE_PRESENT
    //));

    //auto hr = D3DContext::GetCommandList()->Close();
    //if(FAILED(hr)) {
    //    _com_error err(hr);
    //    std::wstring msg = err.ErrorMessage();
    //    DebugBreak();
    //}

    //ID3D12CommandList* cmdLists[] = {D3DContext::GetCommandList().Get()};
    //D3DContext::GetCommandQueue()->ExecuteCommandLists(1, cmdLists);

    //if (FAILED(D3DContext::GetCommandQueue()->Signal(D3DContext::GetFence().Get(), D3DContext::GetCurrentFenceIndex()))) {
    //    DebugBreak();
    //}

    //D3DContext::GetSwapChain()->Present(0, 0);


    //D3DContext::FlushCommandQueue();

}



