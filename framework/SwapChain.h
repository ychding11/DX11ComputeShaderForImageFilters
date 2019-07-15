//=================================================================================================
//
//  MJP's DX12 Sample Framework
// 
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "PCH.h"

namespace SimpleFramework
{

class SwapChain
{

public:

    SwapChain();
    ~SwapChain();

    void Initialize(HWND outputWindow);
    void Shutdown();
    void Reset();

	IDXGISwapChain* D3DSwapChain() const { return swapChain; };
    ID3D11RenderTargetView*const *  RTV() const { return backBuffers; };

    DXGI_FORMAT Format() const { return format; };
    uint32 Width() const { return width; };
    uint32 Height() const { return height; };
    bool FullScreen() const { return fullScreen; };
    bool VSYNCEnabled() const { return vsync; };
    uint32 NumVSYNCIntervals() const { return vsync ? numVSYNCIntervals : 0; };

    // Setters
    void SetFormat(DXGI_FORMAT format_) { format = format_; };
    void SetWidth(uint32 width_) { width = width_; };
    void SetHeight(uint32 height_) { height = height_; };
    void SetFullScreen(bool enabled) { fullScreen = enabled; };
    void SetVSYNCEnabled(bool enabled) { vsync = enabled; };
    void SetNumVSYNCIntervals(uint32 intervals) { numVSYNCIntervals = intervals; };

protected:

    void CheckForSuitableOutput();
    void CreateRTVs();
    void PrepareFullScreenSettings();

	IDXGISwapChain* swapChain = nullptr;

    static const uint64 NumBackBuffers = 1;
	ID3D11RenderTargetView* backBuffers[NumBackBuffers] = {nullptr};

    IDXGIOutput* output = nullptr;

    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT noSRGBFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    uint32 width = 1280;
    uint32 height = 720;
    bool fullScreen = false;
    bool vsync = true;
    DXGI_RATIONAL refreshRate = { };
    uint32 numVSYNCIntervals = 1;
};

}