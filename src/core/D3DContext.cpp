//
// Created by RPG59 on 9/20/2020.
//

#include "D3DContext.h"
// #include "d3dx12.h"

D3DContext* D3DContext::instance = nullptr;

void D3DContext::Init(HWND hWindow) {
	HRESULT hr;
	// #ifndef NDEBUG
	ComPtr<ID3D12Debug3> debugController;
	hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));


	if (FAILED(hr)) {
		DebugBreak();
	}
	debugController->EnableDebugLayer();
	debugController->SetEnableGPUBasedValidation(true);
	// #endif

	ComPtr<IDXGIFactory3> tmpFactory{};
	ComPtr<ID3D12Device> tmpDevice{};

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(tmpFactory.GetAddressOf())));

	auto hr_factory7 = tmpFactory->QueryInterface(__uuidof(IDXGIFactory7), (void**)m_DxgiFactory.GetAddressOf());
	if (FAILED(hr_factory7)) {
		DebugBreak();
	}

	ThrowIfFailed(D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_12_0,
		//            IID_PPV_ARGS(&m_Device)
		//            tmpDevice.GetAddressOf()
		IID_PPV_ARGS(tmpDevice.GetAddressOf())));

	auto hr_device6 = tmpDevice->QueryInterface(__uuidof(ID3D12Device6), (void**)m_Device.GetAddressOf());
	if (FAILED(hr_device6)) {
		DebugBreak();
	}


	//D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels{};
	//msQualityLevels.Format = m_BackBufferFormat;
	//msQualityLevels.SampleCount = 4;
	//msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	//msQualityLevels.NumQualityLevels = 0;
	//ThrowIfFailed(m_Device->CheckFeatureSupport(
	//	D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
	//	&msQualityLevels,
	//	sizeof(msQualityLevels)));

	//m_4xMsaaQuality = msQualityLevels.NumQualityLevels;

#ifndef NDEBUG
	LogAdapters();
#endif

	// commad queue
	D3D12_COMMAND_QUEUE_DESC qd{};
	qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	qd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_Device->CreateCommandQueue(&qd, IID_PPV_ARGS(&m_CommandQueue)));
	// command queue end 

	// swap chain
	CreateSwapChain(hWindow);
	// swap chain end

	CreateRtvAndDsvDescriptorHeaps();


	m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DsvDsecriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_CbvSrvUavDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// swap chain buffrs
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < SwapChainBufferCount; ++i) {
		ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_SwapChainBuffer[i].GetAddressOf())));
		m_Device->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(m_RtvDescriptorSize);
	}
	// swap chain buffrs end

	// command allocators 
	for (int i = 0; i < SwapChainBufferCount; ++i) {
		ThrowIfFailed(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocator[i].GetAddressOf())));
	}
	// command allocators end

	// command list 
	m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator[m_frameIndex].Get(), NULL, IID_PPV_ARGS(m_CommandList.GetAddressOf()));
	// command list end 

	// fence 
	for (int i = 0; i < SwapChainBufferCount; ++i) {
		ThrowIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence[i].GetAddressOf())));
		m_FenceValue[i] = 0;
	}
	// fence end 

	// handle for fence event 
	m_FenceEvent = CreateEvent(nullptr, false, false, nullptr);
	if (m_FenceEvent == nullptr) {
		DebugBreak();
		return;
	}
	// handle for fence event end 


	//root signature
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* signature;
	if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr))) {
		DebugBreak();
	}

	if (FAILED(m_Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.GetAddressOf())))) {
		DebugBreak();
	}
	//root signature end


	// SHADERS
	ID3DBlob* vsShader = nullptr;
	ID3DBlob* errorBuff = nullptr;

	if (FAILED(D3DCompileFromFile(
		L"../shaders/VS.hlsl",
		nullptr,
		nullptr,
		"main",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vsShader,
		&errorBuff
	))) {
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		DebugBreak();
	}

	D3D12_SHADER_BYTECODE vsBytecode{};
	vsBytecode.BytecodeLength = vsShader->GetBufferSize();
	vsBytecode.pShaderBytecode = vsShader->GetBufferPointer();

	ID3DBlob* psShader;

	if (FAILED(D3DCompileFromFile(
		L"../shaders/PS.hlsl",
		nullptr,
		nullptr,
		"main",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&psShader,
		&errorBuff
	))) {
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		DebugBreak();
	}

	D3D12_SHADER_BYTECODE psBytecode{};
	psBytecode.BytecodeLength = psShader->GetBufferSize();
	psBytecode.pShaderBytecode = psShader->GetBufferPointer();

	D3D12_INPUT_ELEMENT_DESC inputLayout[]{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	D3D12_INPUT_LAYOUT_DESC ild{};
	ild.NumElements = _countof(inputLayout);
	ild.pInputElementDescs = inputLayout;

	const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
	{ D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };

	D3D12_DEPTH_STENCIL_DESC dsd{};
	dsd.DepthEnable = true;
	dsd.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	dsd.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	dsd.StencilEnable = false;
	dsd.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	dsd.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	dsd.FrontFace = defaultStencilOp;
	dsd.BackFace = defaultStencilOp;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = ild;
	psoDesc.pRootSignature = m_RootSignature.Get();
	psoDesc.VS = vsBytecode;
	psoDesc.PS = psBytecode;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.NumRenderTargets = 1;
	psoDesc.DepthStencilState = dsd;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	ThrowIfFailed(m_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_PipelineState.GetAddressOf())));

	float vertices[] = {
		-0.5f, 0.5f, 0.5f, 1., 0, 0, 1.,
		0.5f, -0.5f, 0.5f, 0, 0, 0, 1.,
		-0.5f, -0.5f, 0.5f, 0, 0, 1., 1.,
		0.5f, 0.5f, 0.5f, 1., 0, 1., 1.,
		  -0.75f,  0.75f,  0.7f, 0.0f, 1.0f, 0.0f, 1.0f ,
  0.0f,  0.0f, 0.7f, 0.0f, 1.0f, 0.0f, 1.0f ,
	 -0.75f,  0.0f, 0.7f, 0.0f, 1.0f, 0.0f, 1.0f ,
   0.0f,  0.75f,  0.7f, 0.0f, 1.0f, 0.0f, 1.0f
	};

	uint32_t indices[] = {
			0, 1, 2,
			0, 3, 1,

	};

	uint32_t vBufferSize = sizeof(vertices);

	ThrowIfFailed(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(m_VertexBuffer.GetAddressOf())
	));

	m_VertexBuffer->SetName(L"Vertex Buffer Resource Heap");

	ID3D12Resource* vBufferUploadHeap = nullptr;
	ThrowIfFailed(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vBufferUploadHeap)));
	vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	D3D12_SUBRESOURCE_DATA vertexData{};
	vertexData.pData = reinterpret_cast<BYTE*>(vertices);
	vertexData.RowPitch = vBufferSize;
	vertexData.SlicePitch = vBufferSize;

	UpdateSubresources(m_CommandList.Get(), m_VertexBuffer.Get(), vBufferUploadHeap, 0, 0, 1, &vertexData);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
	));

	// Init index buffer
	int indexBufferSize = sizeof(indices);

	m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_IndexBuffer)
	);

	m_VertexBuffer->SetName(L"Index Buffer Resource Heap");

	ID3D12Resource* indexBufferUploadHeap;
	m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBufferUploadHeap)
	);

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<BYTE*>(indices);
	indexData.RowPitch = indexBufferSize;
	indexData.SlicePitch = indexBufferSize;

	UpdateSubresources(m_CommandList.Get(), m_IndexBuffer.Get(), indexBufferUploadHeap, 0, 0, 1, &indexData);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
	m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_IndexBufferView.SizeInBytes = indexBufferSize;
	// init index buffer end 

	m_CommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	m_FenceValue[m_frameIndex]++;

	ThrowIfFailed(m_CommandQueue->Signal(m_Fence[m_frameIndex].Get(), m_FenceValue[m_frameIndex]));

	//SHADERS end

	// create VBO
	m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.StrideInBytes = 7 * 4;
	m_VertexBufferView.SizeInBytes = vBufferSize;
	// create VBO end

	// viewport 
	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(WIDTH);
	m_ScreenViewport.Height = static_cast<float>(HEIGHT);
	m_ScreenViewport.MinDepth = 0;
	m_ScreenViewport.MaxDepth = 1.;
	// viewport end

	// scissor
	m_ScissorTest = { 0, 0, WIDTH, HEIGHT };
	// scissor end
	
	CreateDepthStencilBuffer();

}

void D3DContext::CreateSwapChain(HWND hWindow) {
	m_SwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferDesc.Width = WIDTH;
	sd.BufferDesc.Height = HEIGHT;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = m_BackBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = hWindow;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	IDXGISwapChain* tmpSwapCahin = nullptr;

	m_DxgiFactory->CreateSwapChain(m_CommandQueue.Get(), &sd, &tmpSwapCahin);

	m_SwapChain = static_cast<IDXGISwapChain3*>(tmpSwapCahin);

	m_frameIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

void D3DContext::CreateRtvAndDsvDescriptorHeaps() {
	D3D12_DESCRIPTOR_HEAP_DESC rhd_rtv{};
	rhd_rtv.NumDescriptors = SwapChainBufferCount;
	rhd_rtv.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rhd_rtv.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rhd_rtv.NodeMask = 0;

	ThrowIfFailed(m_Device->CreateDescriptorHeap(&rhd_rtv, IID_PPV_ARGS(m_RtvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC rhd_dsv{};
	rhd_dsv.NumDescriptors = 1;
	rhd_dsv.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	rhd_dsv.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rhd_dsv.NodeMask = 0;

	ThrowIfFailed(m_Device->CreateDescriptorHeap(&rhd_dsv, IID_PPV_ARGS(m_DsvHeap.GetAddressOf())));
}

void D3DContext::LogAdapters() {
	uint32_t i = 0;
	IDXGIAdapter* pAdapter = nullptr;
	std::vector<IDXGIAdapter*> adapterList;

	while (m_DxgiFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND) {
		DXGI_ADAPTER_DESC desc;
		pAdapter->GetDesc(&desc);

		std::wstring adapterName = L"**Adapter: ";
		adapterName += desc.Description;
		adapterName += L"\n";

		// OutputDebugString((adapterName.c_str());

		adapterList.push_back(pAdapter);

		++i;
	}

	for (size_t i = 0; i < adapterList.size(); ++i) {
		ReleaseCom(adapterList[i]);
	}
}

void D3DContext::CreateDepthStencilBuffer() {
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvd{};
	dsvd.Format = DXGI_FORMAT_D32_FLOAT;
	dsvd.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvd.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedCliarValue{};
	depthOptimizedCliarValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedCliarValue.DepthStencil.Depth = 1.0;
	depthOptimizedCliarValue.DepthStencil.Stencil = 0;

	m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, WIDTH, HEIGHT, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedCliarValue,
		IID_PPV_ARGS(&m_DepthStencilBuffer)
	);

	m_DsvHeap->SetName(L"Depth/Stencil Resource Map");
	m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &dsvd, m_DsvHeap->GetCPUDescriptorHandleForHeapStart());
}

void D3DContext::UpdatePipeline() {
	WaitForPrevFrame();

	ThrowIfFailed(m_CommandAllocator[m_frameIndex]->Reset());
	ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator[m_frameIndex].Get(), m_PipelineState.Get()));

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_SwapChainBuffer[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_RtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_DsvHeap->GetCPUDescriptorHandleForHeapStart());
	m_CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	const float color[4] = { .3, .4, .1, .0 };
	m_CommandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
	m_CommandList->ClearDepthStencilView(m_DsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1., 0, 0, nullptr);


	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
	m_CommandList->RSSetViewports(1, &m_ScreenViewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorTest);
	m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_CommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	m_CommandList->IASetIndexBuffer(&m_IndexBufferView);
	//m_CommandList->DrawInstanced(3, 1, 0, 0);
	m_CommandList->DrawIndexedInstanced(6, 1, 0, 4, 0);
	m_CommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);


	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_SwapChainBuffer[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(m_CommandList->Close());

}

void D3DContext::Render() {
	UpdatePipeline();

	ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };

	m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	ThrowIfFailed(m_CommandQueue->Signal(m_Fence[m_frameIndex].Get(), m_FenceValue[m_frameIndex]));
	ThrowIfFailed(m_SwapChain->Present(0, 0));
}

void D3DContext::WaitForPrevFrame() {
	m_frameIndex = m_SwapChain->GetCurrentBackBufferIndex();

	if (m_Fence[m_frameIndex]->GetCompletedValue() < m_FenceValue[m_frameIndex]) {
		ThrowIfFailed(m_Fence[m_frameIndex]->SetEventOnCompletion(m_FenceValue[m_frameIndex], m_FenceEvent));

		WaitForSingleObject(m_FenceEvent, INFINITE);
	}

	m_FenceValue[m_frameIndex]++;
}








