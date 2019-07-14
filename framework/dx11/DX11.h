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

namespace DX11
{

// Constants
const uint64 RenderLatency = 2;

// Externals
extern D3D_FEATURE_LEVEL FeatureLevel;
extern IDXGIFactory1* Factory;  // For DX11
extern IDXGIAdapter1* Adapter;

extern uint64 CurrentCPUFrame;  // Total number of CPU frames completed (completed means all command buffers submitted to the GPU)
extern uint64 CurrentGPUFrame;  // Total number of GPU frames completed (completed means that the GPU signals the fence)
extern uint64 CurrFrameIdx;     // CurrentCPUFrame % RenderLatency

// Lifetime
void Initialize(D3D_FEATURE_LEVEL minFeatureLevel, uint32 adapterIdx);
void Shutdown();

ID3D11Device* Device();
ID3D11DeviceContext* ImmediateContext();

void DeferredRelease_(IUnknown* resource);

template<typename T> void DeferredRelease(T*& resource)
{
    IUnknown* base = resource;
    DeferredRelease_(base);
    resource = nullptr;
}

template<typename T> void Release(T*& resource)
{
    if(resource != nullptr) {
        resource->Release();
        resource = nullptr;
    }
}


} // namespace
} // namespace


