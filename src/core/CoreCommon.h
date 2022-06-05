#pragma once

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <comdef.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "../../dep/include/d3dx12.h"

#include <memory>
#include <functional>
#include <cstdint>
#include <vector>
#include <string>

#include "../Util.h"

#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

using Microsoft::WRL::ComPtr;
using namespace DirectX;

extern HWND g_hwnd;
extern uint32_t g_currentBuffer;

const uint32_t g_displayWidth = 1360;
const uint32_t g_displayHeight = 768;