
#pragma once

#include "PCH.h"
#include "Timer.h"
#include "Window.h"
#include "SwapChain.h"

namespace SimpleFramework
{

class Window;

class App
{
public:

    App(const wchar* appName, const wchar* iconResource = NULL);
    virtual ~App();

    int32 Run();

protected:

    virtual void Update(const Timer& timer) = 0;
    virtual void Render(const Timer& timer) = 0;


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

private:

    void Initialize_private();
	void BeginFrame_private();
	void EndFrame_private();
	

public:

    // Accessors
    Window& Window() { return window; }
};

extern App* GlobalApp;

}