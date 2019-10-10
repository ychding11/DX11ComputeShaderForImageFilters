//=================================================================================================
//
// 
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "PCH.h"

#include "GHIResources.h" 
#include "GHICommandContext.h" 

namespace GHI
{

    class SwapChain
    {
    public:

        // should be virtual function: platform dependent
        virtual void Initialize(HWND outputWindow) = 0;
        virtual void Shutdown() = 0;
        virtual void Reset() =0;

        virtual bool Present() = 0;
        virtual GHITexture** ColorBuffers() const = 0;


        uint32 Width() const { return width; };
        uint32 Height() const { return height; };

        void SetWidth(uint32 width_) { width = width_; };
        void SetHeight(uint32 height_) { height = height_; };

        bool FullScreen() const { return fullScreen; };
        bool VSYNCEnabled() const { return vsync; };
        uint32 NumVSYNCIntervals() const { return vsync ? numVSYNCIntervals : 0; };

        void SetFullScreen(bool enabled) { fullScreen = enabled; };
        void SetVSYNCEnabled(bool enabled) { vsync = enabled; };
        void SetNumVSYNCIntervals(uint32 intervals) { numVSYNCIntervals = intervals; };

    protected:

        uint32 width = 1280;
        uint32 height = 720;
        bool fullScreen = false;
        bool vsync = true;
        uint32 numVSYNCIntervals = 1;
    };

    class SwapChainDX11 :public SwapChain
    {
    public:
        SwapChainDX11();

        virtual ~SwapChainDX11();
        
        virtual void Initialize(HWND outputWindow) override;
        virtual void Shutdown() override;
        virtual void Reset() override;

        virtual bool Present() override
        {
            return swapChain->Present(0,0);
        }
        virtual GHITexture** ColorBuffers() const override
        {
            //return &buffers[0]; // hardcoded, only one buffer accessiable 
            return const_cast<GHITexture**>(buffers);
        }

    private:
        void PrepareFullScreenSettings();
        void CreateBuffers();

	    IDXGISwapChain* swapChain = nullptr;

        static const uint64 NumBackBuffers = 1;
        GHITexture *buffers[NumBackBuffers] = { nullptr };

        IDXGIOutput* output = nullptr;
        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
        DXGI_FORMAT noSRGBFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        DXGI_RATIONAL refreshRate = { };
    };

}