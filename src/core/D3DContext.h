//
// Created by RPG59 on 9/20/2020.
//
#pragma once

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <comdef.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <cstdint>
#include <string>
#include <vector>

#include "../../dep/include/d3dx12.h"
#include "../Util.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

#define WIDTH 1360
#define HEIGHT 768

struct ConstantBuffer
{
	XMFLOAT4 colorMultiplier;
};

struct ConstantBufferPerObject
{
	XMFLOAT4X4 mvpMatrix;
};

class D3DContext
{
private:
	static const int SwapChainBufferCount = 2;

	ComPtr<ID3D12Device6> m_Device;
	ComPtr<IDXGIFactory4> m_DxgiFactory;
	ComPtr<IDXGISwapChain3> m_SwapChain;

	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;

	ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

	ComPtr<ID3D12Fence> m_Fence[SwapChainBufferCount];
	uint64_t m_FenceValue[SwapChainBufferCount];

	// descriptor sizes can vary across GPUs
	uint32_t m_RtvDescriptorSize = 0;
	uint32_t m_DsvDsecriptorSize = 0;
	uint32_t m_CbvSrvUavDescriptorSize = 0;
	// ********

	DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	//DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	std::wstring m_MainWndCaption = L"d3d App";

	uint32_t m_4xMsaaQuality = 0;

	int m_CurrBackBuffer = 0;

	ComPtr<ID3D12Resource> m_SwapChainBuffer[SwapChainBufferCount];
	ComPtr<ID3D12Resource> m_DepthStencilBuffer;

	D3D12_VIEWPORT m_ScreenViewport{};
	D3D12_RECT m_ScissorTest{};

	ComPtr<ID3D12RootSignature> m_RootSignature;
	ComPtr<ID3D12PipelineState> m_PipelineState;
	ComPtr<ID3D12Resource> m_VertexBuffer;

	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView{};

	int m_frameIndex = 0;

	ComPtr<ID3D12CommandAllocator> m_CommandAllocator[SwapChainBufferCount];
	HANDLE m_FenceEvent;

	ComPtr<ID3D12Resource> m_IndexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

	ComPtr<ID3D12DescriptorHeap> m_MainDescriptorHeap[SwapChainBufferCount];

	ConstantBuffer m_CbColorMultiplierData; // constant buffer data


	ConstantBufferPerObject m_CbPerObjec;

	ComPtr<ID3D12Resource> m_ConstantBufferUploadHeaps[SwapChainBufferCount];

	uint8_t* m_CbGPUAddress[SwapChainBufferCount];

	XMFLOAT4X4 m_ProjectionMatrix;
	XMFLOAT4X4 m_ViewMatrix;

	XMFLOAT4 m_CameraPosition;
	XMFLOAT4 m_CameraTarget;
	XMFLOAT4 m_CameraUp;

	XMFLOAT4X4 m_Cube1WorldMatrix;
	XMFLOAT4X4 m_Cube1RotationMatrix;
	XMFLOAT4 m_Cube1Position;

	XMFLOAT4X4 m_Cube2WorldMatrix;
	XMFLOAT4X4 m_Cube2RotationMatrix;
	XMFLOAT4 m_Cube2PositionOffset;

	int m_NumCubeIndex;
	int m_ConstantBufferPerObjectAlignedSize = (sizeof(ConstantBufferPerObject) + 255) & ~255;
	
private:
	static D3DContext* instance;

public:
	void Init(HWND);

	void CreateSwapChain(HWND);

	void CreateRtvAndDsvDescriptorHeaps();

	void LogAdapters();

	void OnResize();

	void InitShaders();

	void UpdatePipeline();

	void WaitForPrevFrame();

	void Render();

	void Cleanup();

	void CreateDepthStencilBuffer();

	void SetConstantBuffer();

	void UpdateConstantBufferData();

	void Update();

	static void setContext(D3DContext* instance) { D3DContext::instance = instance; }

	static D3DContext* getContext() { return D3DContext::instance; }

	static size_t CurrentBackBufferView();

	static void FlushCommandQueue();


	inline static ComPtr<ID3D12Device6> GetDevice() { return D3DContext::getContext()->m_Device; }

	inline static ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return D3DContext::getContext()->m_CommandList; }


	inline static ComPtr<ID3D12CommandQueue> GetCommandQueue() { return D3DContext::getContext()->m_CommandQueue; }

	inline void D3DCall()
	{
		auto hr = m_Device->GetDeviceRemovedReason();
		if (FAILED(hr))
		{
			_com_error err(hr);
			OutputDebugString(err.ErrorMessage());
			throw;
		}
	}

	//inline static ComPtr<ID3D12Resource> getCurrBackBuffer() { return D3DContext::getContext()->m_SwapChainBuffer[D3DContext::getContext()->m_CurrBackBuffer]; }
	//inline static ComPtr<ID3D12DescriptorHeap> GetRtvHeap() { return D3DContext::getContext()->m_RtvHeap; }
	//inline static ComPtr<IDXGISwapChain3> GetSwapChain() { return D3DContext::getContext()->m_SwapChain; }
	//inline static ComPtr<ID3D12RootSignature> GetRootSignature() { return D3DContext::getContext()->m_RootSignature; }
	//inline static D3D12_VIEWPORT GetViewport() { return D3DContext::getContext()->m_ScreenViewport; }
	//inline static D3D12_RECT GetScissor() { return D3DContext::getContext()->m_ScissorTest; }
	//inline static ComPtr<ID3D12Resource> GetVertexBuffer() { return D3DContext::getContext()->m_VertexBuffer; }
	//inline static D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() { return D3DContext::getContext()->m_VertexBufferView; }
};
