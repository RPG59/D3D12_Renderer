#include "D3DContext.h"
#include "Display.h"

D3DContext *D3DContext::instance = nullptr;

void D3DContext::Init()
{
	HRESULT hr;
	// #ifndef NDEBUG
	ComPtr<ID3D12Debug3> debugController;

	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));

	debugController->EnableDebugLayer();
	debugController->SetEnableGPUBasedValidation(true);
	// #endif

	ComPtr<IDXGIFactory3> tmp_factory{};
	ComPtr<ID3D12Device> tmp_device{};

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(tmp_factory.GetAddressOf())));

	ThrowIfFailed(tmp_factory->QueryInterface(__uuidof(IDXGIFactory7), (void **)m_DxgiFactory.GetAddressOf()));

	ThrowIfFailed(D3D12CreateDevice(
			nullptr,
			D3D_FEATURE_LEVEL_12_0,
			IID_PPV_ARGS(tmp_device.GetAddressOf())));

	ThrowIfFailed(tmp_device->QueryInterface(__uuidof(ID3D12Device6), (void **)m_Device.GetAddressOf()));

	// D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels{};
	// msQualityLevels.Format = m_BackBufferFormat;
	// msQualityLevels.SampleCount = 4;
	// msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	// msQualityLevels.NumQualityLevels = 0;
	// ThrowIfFailed(m_Device->CheckFeatureSupport(
	//	D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
	//	&msQualityLevels,
	//	sizeof(msQualityLevels)));

	// m_4xMsaaQuality = msQualityLevels.NumQualityLevels;

#ifndef NDEBUG
	LogAdapters();
#endif

	// commad queue
	D3D12_COMMAND_QUEUE_DESC qd{};
	qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	qd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_Device->CreateCommandQueue(&qd, IID_PPV_ARGS(&m_CommandQueue)));
	// command queue end

	Display::getInstance()->init();
	auto currentBackBufferIndex = Display::getInstance()->getCurrentBackBufferIndex();

	m_DsvDsecriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_CbvSrvUavDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// command allocators
	for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
	{
		ThrowIfFailed(
				m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocator[i].GetAddressOf())));
	}
	// command allocators end

	// command list
	m_Device->CreateCommandList(0,
															D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator[currentBackBufferIndex].Get(), NULL,
															IID_PPV_ARGS(m_CommandList.GetAddressOf()));
	// command list end

	// fence
	for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
	{
		ThrowIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence[i].GetAddressOf())));
		m_FenceValue[i] = 0;
	}
	// fence end

	// handle for fence event
	m_FenceEvent = CreateEvent(nullptr, false, false, nullptr);
	if (m_FenceEvent == nullptr)
	{
		DebugBreak();
		return;
	}
	// handle for fence event end

	// root signature
	SetConstantBuffer();
	// root signature end

	// SHADERS
	ID3DBlob *vsShader = nullptr;
	ID3DBlob *errorBuff = nullptr;

	if (FAILED(D3DCompileFromFile(
					L"../shaders/VS.hlsl",
					nullptr,
					nullptr,
					"main",
					"vs_5_0",
					D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
					0,
					&vsShader,
					&errorBuff)))
	{
		OutputDebugStringA((char *)errorBuff->GetBufferPointer());
		DebugBreak();
	}

	D3D12_SHADER_BYTECODE vsBytecode{};
	vsBytecode.BytecodeLength = vsShader->GetBufferSize();
	vsBytecode.pShaderBytecode = vsShader->GetBufferPointer();

	ID3DBlob *psShader;

	if (FAILED(D3DCompileFromFile(
					L"../shaders/PS.hlsl",
					nullptr,
					nullptr,
					"main",
					"ps_5_0",
					D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
					0,
					&psShader,
					&errorBuff)))
	{
		OutputDebugStringA((char *)errorBuff->GetBufferPointer());
		DebugBreak();
	}

	D3D12_SHADER_BYTECODE psBytecode{};
	psBytecode.BytecodeLength = psShader->GetBufferSize();
	psBytecode.pShaderBytecode = psShader->GetBufferPointer();

	D3D12_INPUT_ELEMENT_DESC inputLayout[]{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

	D3D12_INPUT_LAYOUT_DESC ild{};
	ild.NumElements = _countof(inputLayout);
	ild.pInputElementDescs = inputLayout;

	const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
			{D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS};

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
			-0.75f, 0.75f, 0.7f, 0.0f, 1.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 0.7f, 0.0f, 1.0f, 0.0f, 1.0f,
			-0.75f, 0.0f, 0.7f, 0.0f, 1.0f, 0.0f, 1.0f,
			0.0f, 0.75f, 0.7f, 0.0f, 1.0f, 0.0f, 1.0f};

	uint32_t indices[] = {
			0,
			1,
			2,
			0,
			3,
			1,

	};

	uint32_t vBufferSize = sizeof(vertices);

	UploadBuffer vertexUploadBuffer = UploadBuffer();
	UploadBuffer indexUploadBuffer = UploadBuffer();

	vertexUploadBuffer.create(L"vertex upload buffer", sizeof(vertices));

	memcpy(vertexUploadBuffer.map(), vertices, sizeof(vertices));
	vertexUploadBuffer.unmap();

	m_VertexBuffer = GpuBuffer();
	m_VertexBuffer.create(L"Vertex Buffer", 56, sizeof(float), vertexUploadBuffer);

	// Index buffer
	indexUploadBuffer.create(L"index upload buffer", sizeof(indices));
	memcpy(indexUploadBuffer.map(), indices, sizeof(indices));
	indexUploadBuffer.unmap();

	m_IndexBuffer = GpuBuffer();
	m_IndexBuffer.create(L"Index buffer", sizeof(indices) / sizeof(float), sizeof(float), indexUploadBuffer);

	// --------------------------------------------------------------
	m_IndexBufferView.BufferLocation = m_IndexBuffer.getGpuVirtualAddress();
	m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_IndexBufferView.SizeInBytes = m_IndexBuffer.getBufferSize();
	// init index buffer end

	m_CommandList->Close();
	ID3D12CommandList *ppCommandLists[] = {m_CommandList.Get()};
	m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	m_FenceValue[currentBackBufferIndex]++;

	ThrowIfFailed(m_CommandQueue->Signal(m_Fence[currentBackBufferIndex].Get(), m_FenceValue[currentBackBufferIndex]));

	// SHADERS end

	// create VBO
	m_VertexBufferView.BufferLocation = m_VertexBuffer.getGpuVirtualAddress();
	m_VertexBufferView.StrideInBytes = 7 * 4;
	m_VertexBufferView.SizeInBytes = m_VertexBuffer.getBufferSize();
	// create VBO end

	// viewport
	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(g_displayWidth);
	m_ScreenViewport.Height = static_cast<float>(g_displayHeight);
	m_ScreenViewport.MinDepth = 0;
	m_ScreenViewport.MaxDepth = 1.;
	// viewport end

	// scissor
	m_ScissorTest = {0, 0, (long)g_displayWidth, (long)g_displayHeight};
	// scissor end

	CreateDepthStencilBuffer();
}

void D3DContext::LogAdapters()
{
	uint32_t i = 0;
	IDXGIAdapter *pAdapter = nullptr;
	std::vector<IDXGIAdapter *> adapterList;

	while (m_DxgiFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		pAdapter->GetDesc(&desc);

		std::wstring adapterName = L"**Adapter: ";
		adapterName += desc.Description;
		adapterName += L"\n";

		OutputDebugString(adapterName.c_str());

		adapterList.push_back(pAdapter);

		++i;
	}

	for (size_t i = 0; i < adapterList.size(); ++i)
	{
		ReleaseCom(adapterList[i]);
	}
}

void D3DContext::CreateDepthStencilBuffer()
{
	UpdateConstantBufferData();
}

void D3DContext::SetGraphicsRootSignature()
{
	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
}

void D3DContext::UpdatePipeline(uint32_t currentBackBufferIndex)
{
	if (m_Fence[currentBackBufferIndex]->GetCompletedValue() < m_FenceValue[currentBackBufferIndex])
	{
		ThrowIfFailed(m_Fence[currentBackBufferIndex]->SetEventOnCompletion(m_FenceValue[currentBackBufferIndex], m_FenceEvent));

		WaitForSingleObject(m_FenceEvent, INFINITE);
	}

	m_FenceValue[currentBackBufferIndex]++;

	ThrowIfFailed(m_CommandAllocator[currentBackBufferIndex]->Reset());
	ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator[currentBackBufferIndex].Get(), m_PipelineState.Get()));

	Display::getInstance()->prepearePresent();

	ThrowIfFailed(m_CommandList->Close());
}

void D3DContext::SetBuffers()
{
	auto currentBackBufferIndex = Display::getInstance()->getCurrentBackBufferIndex();

	m_CommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	m_CommandList->IASetIndexBuffer(&m_IndexBufferView);

	m_CommandList->SetGraphicsRootConstantBufferView(0, m_ConstantBufferUploadHeaps[currentBackBufferIndex]->GetGPUVirtualAddress());
	m_CommandList->DrawIndexedInstanced(6, 1, 0, 4, 0);

	m_CommandList->SetGraphicsRootConstantBufferView(0, m_ConstantBufferUploadHeaps[currentBackBufferIndex]->GetGPUVirtualAddress() + m_ConstantBufferPerObjectAlignedSize);
	m_CommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void D3DContext::SetScissor()
{
	m_CommandList->RSSetScissorRects(1, &m_ScissorTest);
}

void D3DContext::SetViewport()
{
	m_CommandList->RSSetViewports(1, &m_ScreenViewport);
}

void D3DContext::Render()
{
	auto currentBackBufferIndex = Display::getInstance()->getCurrentBackBufferIndex();

	UpdatePipeline(currentBackBufferIndex);

	ID3D12CommandList *ppCommandLists[] = {m_CommandList.Get()};

	m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	ThrowIfFailed(m_CommandQueue->Signal(m_Fence[currentBackBufferIndex].Get(), m_FenceValue[currentBackBufferIndex]));
	// ThrowIfFailed(m_SwapChain->Present(0, 0));
	Display::getInstance()->present();
}

void D3DContext::SetConstantBuffer()
{
	D3D12_ROOT_DESCRIPTOR rootCBVDescriptor{};
	rootCBVDescriptor.ShaderRegister = 0;
	rootCBVDescriptor.RegisterSpace = 0;

	D3D12_ROOT_PARAMETER rp[1];
	rp[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rp[0].Descriptor = rootCBVDescriptor;
	rp[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(
			1,
			rp,
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
					D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);

	ID3DBlob *signature;
	if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr)))
	{
		DebugBreak();
	}

	if (FAILED(
					m_Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_RootSignature.GetAddressOf()))))
	{
		DebugBreak();
	}
}

void D3DContext::UpdateConstantBufferData()
{
	for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
	{
		auto constantBufferUploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto constantBufferUploadHeapDesc =
				CD3DX12_RESOURCE_DESC::Buffer(1024 * 64);
		ThrowIfFailed(m_Device->CreateCommittedResource(
				&constantBufferUploadHeapProps,
				D3D12_HEAP_FLAG_NONE,
				&constantBufferUploadHeapDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(m_ConstantBufferUploadHeaps[i].GetAddressOf())));
		m_ConstantBufferUploadHeaps[i]->SetName(L"Constant buffer upload heap");

		m_CbPerObjec = {};

		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(m_ConstantBufferUploadHeaps[i]->Map(0, &readRange, reinterpret_cast<void **>(&m_CbGPUAddress[i])));
		memcpy((m_CbGPUAddress[i]), &m_CbPerObjec, sizeof(m_CbPerObjec));
		memcpy(m_CbGPUAddress[i] + m_ConstantBufferPerObjectAlignedSize, &m_CbPerObjec, sizeof(m_CbPerObjec));
	}

	XMMATRIX tmpMat = XMMatrixPerspectiveFovLH(45. * (XM_PI / 180.), (float)g_displayWidth / (float)g_displayHeight, .1, 1000.);
	XMStoreFloat4x4(&m_ProjectionMatrix, tmpMat);

	m_CameraPosition = XMFLOAT4(.0, 2., -4., .0);
	m_CameraTarget = XMFLOAT4(.0, .0, .0, .0);
	m_CameraUp = XMFLOAT4(0, 1., 0., 0.);

	tmpMat = XMMatrixLookAtLH(
			XMLoadFloat4(&m_CameraPosition),
			XMLoadFloat4(&m_CameraTarget),
			XMLoadFloat4(&m_CameraUp));
	XMStoreFloat4x4(&m_ViewMatrix, tmpMat);

	m_Cube1Position = XMFLOAT4(.0, .0, .0, .0);
	XMVECTOR posVec = XMLoadFloat4(&m_Cube1Position);

	tmpMat = XMMatrixTranslationFromVector(posVec);
	XMStoreFloat4x4(&m_Cube1RotationMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_Cube1WorldMatrix, tmpMat);

	m_Cube2PositionOffset = XMFLOAT4(1.5, .0, .0, .0);
	posVec = XMLoadFloat4(&m_Cube2PositionOffset);

	tmpMat = XMMatrixTranslationFromVector(posVec);
	XMStoreFloat4x4(&m_Cube2RotationMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_Cube2WorldMatrix, tmpMat);
}

void D3DContext::Update()
{
	auto currentBackBufferIndex = Display::getInstance()->getCurrentBackBufferIndex();
	XMMATRIX rotXmat = XMMatrixRotationX(.0001);
	XMMATRIX rotYmat = XMMatrixRotationY(.0002);
	XMMATRIX rotZmat = XMMatrixRotationZ(.0003);

	XMMATRIX rotMat = XMLoadFloat4x4(&m_Cube1RotationMatrix) * rotXmat * rotYmat * rotZmat;
	XMStoreFloat4x4(&m_Cube1RotationMatrix, rotMat);

	XMMATRIX translationMat = XMMatrixTranslationFromVector(XMLoadFloat4(&m_Cube1Position));
	XMMATRIX worldMat = rotMat * translationMat;
	XMStoreFloat4x4(&m_Cube1WorldMatrix, worldMat);

	XMMATRIX viewMat = XMLoadFloat4x4(&m_ViewMatrix);
	XMMATRIX porjMat = XMLoadFloat4x4(&m_ProjectionMatrix);
	XMMATRIX mvpMat = XMLoadFloat4x4(&m_Cube1WorldMatrix) * viewMat * porjMat;
	XMMATRIX transposed = XMMatrixTranspose(mvpMat);
	XMStoreFloat4x4(&m_CbPerObjec.mvpMatrix, transposed);

	memcpy(m_CbGPUAddress[currentBackBufferIndex], &m_CbPerObjec, sizeof(m_CbPerObjec));

	rotXmat = XMMatrixRotationX(.0003);
	rotYmat = XMMatrixRotationY(.0002);
	rotZmat = XMMatrixRotationZ(.0001);

	rotMat = rotZmat * (XMLoadFloat4x4(&m_Cube2RotationMatrix) * (rotXmat * rotYmat));
	XMStoreFloat4x4(&m_Cube2RotationMatrix, rotMat);

	XMMATRIX translationOffsetMat = XMMatrixTranslationFromVector(XMLoadFloat4(&m_Cube2PositionOffset));

	XMMATRIX scaleMat = XMMatrixScaling(.5, .5, .5);

	worldMat = scaleMat * translationOffsetMat * rotMat * translationMat;
	mvpMat = XMLoadFloat4x4(&m_Cube2WorldMatrix) * viewMat * porjMat;
	transposed = XMMatrixTranspose(mvpMat);
	XMStoreFloat4x4(&m_CbPerObjec.mvpMatrix, transposed);

	memcpy(m_CbGPUAddress[currentBackBufferIndex] + m_ConstantBufferPerObjectAlignedSize, &m_CbPerObjec, sizeof(m_CbPerObjec));

	XMStoreFloat4x4(&m_Cube2WorldMatrix, worldMat);
}
