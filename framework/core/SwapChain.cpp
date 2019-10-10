//=================================================================================================
//
// 
//
//  All code licensed under the MIT license
//
//=================================================================================================

#include "PCH.h"
#include "DX11.h"

#include "SwapChain.h"
#include "Exceptions.h"

#include "FDX11GHIResources.h" 

using std::wstring;

namespace GHI
{

    SwapChainDX11::SwapChainDX11()
    {
        refreshRate.Numerator = 60;
        refreshRate.Denominator = 1;

        // Try to figure out should default to 1280x720 or 1920x1080
        POINT point;
        point.x = 0;
        point.y = 0;
        HMONITOR monitor = MonitorFromPoint(point, MONITOR_DEFAULTTOPRIMARY);
        if(monitor != 0)
        {
            MONITORINFOEX info;
            ZeroMemory(&info, sizeof(info));
            info.cbSize = sizeof(MONITORINFOEX);
            if(GetMonitorInfo(monitor, &info) != 0)
            {
                int32 monitorWidth = info.rcWork.right - info.rcWork.left;
                int32 monitorHeight = info.rcWork.bottom - info.rcWork.top;
                if(monitorWidth > 1920 && monitorHeight > 1080)
                {
                    width = 1920;
                    height = 1080;
                }
            }
        }
    }

    SwapChainDX11::~SwapChainDX11()
    {
        Assert_(swapChain == nullptr);
        Shutdown();
    }

    void SwapChainDX11::PrepareFullScreenSettings()
    {
        Assert_(output);

        // Have the Output look for the closest matching mode
        DXGI_MODE_DESC desiredMode;
        desiredMode.Format = noSRGBFormat;
        desiredMode.Width = width;
        desiredMode.Height = height;
        desiredMode.RefreshRate.Numerator = 0;
        desiredMode.RefreshRate.Denominator = 0;
        desiredMode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        desiredMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

        DXGI_MODE_DESC closestMatch;
        DXCall(output->FindClosestMatchingMode(&desiredMode, &closestMatch, DX11::Device()));

        width = closestMatch.Width;
        height = closestMatch.Height;
        refreshRate = closestMatch.RefreshRate;
    }

    /* Initialize swapchain */
    void SwapChainDX11::Initialize(HWND outputWindow)
    {
        Shutdown();

        // We'll just use the first output for fullscreen
        DX11::Adapter->EnumOutputs(0, &output);

        if (format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
            noSRGBFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        else if (format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)
            noSRGBFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
        else
            noSRGBFormat = format;

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        swapChainDesc.BufferCount = uint32(NumBackBuffers);
        swapChainDesc.BufferDesc.Width = width;
        swapChainDesc.BufferDesc.Height = height;
        swapChainDesc.BufferDesc.Format = noSRGBFormat;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // only 1 buffer can be accessed.
        swapChainDesc.OutputWindow = outputWindow;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.Windowed = TRUE;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        IDXGISwapChain* tempSwapChain = nullptr;
        DXCall(DX11::Factory->CreateSwapChain(DX11::Device(), &swapChainDesc, &tempSwapChain));
        DXCall(tempSwapChain->QueryInterface(IID_PPV_ARGS(&swapChain)));
        DX11::Release(tempSwapChain);

        CreateBuffers();
    }

    void SwapChainDX11::Shutdown()
    {
        for (uint64 i = 0; i < NumBackBuffers; ++i)
        {
            delete (buffers[i]);
        }

        DX11::Release(swapChain);
        DX11::Release(output);
    }

    void SwapChainDX11::Reset()
    {
        Assert_(swapChain);
        if(output == nullptr)
            fullScreen = false;

        // Release all references
        for(uint64 i = 0; i < NumBackBuffers; ++i)
        {
            delete (buffers[i]);
        }

        if(format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
            noSRGBFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        else if(format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)
            noSRGBFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
        else
            noSRGBFormat = format;

        if(fullScreen)
            PrepareFullScreenSettings();
        else
        {
            refreshRate.Numerator = 60;
            refreshRate.Denominator = 1;
        }

        DXCall(swapChain->SetFullscreenState(fullScreen, NULL));

        DXCall(swapChain->ResizeBuffers(NumBackBuffers, width, height, noSRGBFormat,
                                        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH |
                                        DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING |
                                        DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT));

        if(fullScreen)
        {
            DXGI_MODE_DESC mode;
            mode.Format = noSRGBFormat;
            mode.Width = width;
            mode.Height = height;
            mode.RefreshRate.Numerator = 0;
            mode.RefreshRate.Denominator = 0;
            mode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            mode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            DXCall(swapChain->ResizeTarget(&mode));
        }

	    CreateBuffers();
    }


    void SwapChainDX11::CreateBuffers()
    {
        // Re-create an RTV for each back buffer
        for(uint64 i = 0; i < NumBackBuffers; i++)
        {
		    ID3D11Texture2D* pBackBuffer = nullptr;
            ID3D11RenderTargetView *pBufferView = nullptr;

            // DXGI_SWAP_EFFECT_DISCARD only access one buffer, so always use 0.
		    DXCall(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer));
            DXCall(DX11::Device()->CreateRenderTargetView(pBackBuffer, nullptr, &pBufferView));

            if (buffers[i] != nullptr)
            {
                delete buffers[i];
            }
            FDX11GHITexture *tex = new FDX11GHITexture(pBackBuffer);
            tex->rawRTV = pBufferView;
            buffers[i] = tex;
        }
    }

}