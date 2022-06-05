#pragma once

#include <windows.h>
#include <string>
#include <array>
#include <cassert>

#define SWAP_CHAIN_BUFFER_COUNT 2

inline std::wstring AnsiToWString(const std::string &str)
{
    WCHAR buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
    return std::wstring(buffer);
}

class DxException
{
public:
    DxException() = default;

    DxException(HRESULT, const std::wstring &, const std::wstring &, int);

    std::wstring ToString() const;

    HRESULT ErrorCode = S_OK;
    std::wstring FunctionName;
    std::wstring Filename;
    int LineNumber = -1;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                      \
    {                                                         \
        HRESULT hr__ = (x);                                   \
        std::wstring wfn = AnsiToWString(__FILE__);           \
        if (FAILED(hr__))                                     \
        {                                                     \
            auto err = DxException(hr__, L#x, wfn, __LINE__); \
            throw err;                                        \
        }                                                     \
    }
#endif

#ifndef ReleaseCom
#define ReleaseCom(x)     \
    {                     \
        if (x)            \
        {                 \
            x->Release(); \
            x = 0;        \
        }                 \
    }
#endif
