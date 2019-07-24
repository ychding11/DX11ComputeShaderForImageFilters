
#pragma once

#include "PCH.h"
#include "Timer.h"
#include "Utility.h"
#include "Window.h"
#include "SwapChain.h"
#include "GHIResources.h"
#include "GHICommandContext.h"

namespace SimpleFramework
{

class Window;

class App
{
public:

    App(const wchar* appName=L"sample", const wchar* iconResource = NULL);
    virtual ~App();

    int32 Run();

private:
    void Initialize_private();
	void BeginFrame_private();
	void EndFrame_private();
	void Shutdown_private();
	
protected:

    virtual void Initialize() = 0;
    virtual void Update(const Timer& timer) = 0;
    virtual void Render(const Timer& timer) = 0;
    virtual void Shutdown() = 0;


	ID3D11Device* Device();
	ID3D11DeviceContext* ImmediateContext();

	uint32 SwapchainWidth() const;
	uint32 SwapchainHeight() const;

    void Exit();
    void ToggleFullScreen(bool fullScreen);
    void CalculateFPS();

    static void OnWindowResized(void* context, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    Window window;
	SwapChain swapchain;
    Timer timer;

    static const uint32 NumTimeDeltaSamples = 64;
    float timeDeltaBuffer[NumTimeDeltaSamples];
    uint32 currentTimeDeltaSample;
    uint32 fps;

    std::wstring applicationName;
    std::string globalHelpText = "Simple framework for DX11";

    bool createConsole;
    bool showWindow;
    int32 returnCode;

	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

public:

    // Accessors
    Window& Window() { return window; }
	std::string Name() const;

protected:

    IGHIComputeCommandCotext *commandContext = nullptr;
	GHISampler *linearSampler = nullptr;
    //std::unique_ptr<GHISampler> linearSampler;

    inline std::wstring ToWstr(const std::string &str)
    {
        return StrToWstring(str.c_str());
    }
    inline std::string ToStr(const std::wstring &wstr)
    {
        return WstringToStr(wstr.c_str());
    }
};

extern App* GlobalApp;

}