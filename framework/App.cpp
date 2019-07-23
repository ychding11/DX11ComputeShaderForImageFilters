//=================================================================================================
//
//  
// 
//
//  All code licensed under the MIT license
//
//=================================================================================================

#include "PCH.h"
#include "DX11.h"
#include "App.h"
#include "Utility.h"
#include "FDX11GHICommandContext.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

namespace SimpleFramework
{
	namespace imgui
	{
		static void WindowMessageCallback(void* context, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			ImGuiIO& io = ImGui::GetIO();
			switch (msg)
			{
			case WM_LBUTTONDOWN:
				io.MouseDown[0] = true;
				return;
			case WM_LBUTTONUP:
				io.MouseDown[0] = false;
				return;
			case WM_RBUTTONDOWN:
				io.MouseDown[1] = true;
				return;
			case WM_RBUTTONUP:
				io.MouseDown[1] = false;
				return;
			case WM_MBUTTONDOWN:
				io.MouseDown[2] = true;
				return;
			case WM_MBUTTONUP:
				io.MouseDown[2] = false;
				return;
			case WM_MOUSEWHEEL:
				io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
				return;
			case WM_MOUSEMOVE:
				io.MousePos.x = (signed short)(lParam);
				io.MousePos.y = (signed short)(lParam >> 16);
				return;
			case WM_KEYDOWN:
				if (wParam < 256)
					io.KeysDown[wParam] = 1;
				return;
			case WM_KEYUP:
				if (wParam < 256)
					io.KeysDown[wParam] = 0;
				return;
			case WM_CHAR:
				if (wParam > 0 && wParam < 0x10000)
					io.AddInputCharacter(uint16(wParam));
				return;
			}
		}

		void Initialize(Window& window)
		{

			// Setup Dear ImGui context
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

			io.KeyMap[ImGuiKey_Tab] = VK_TAB;
			io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
			io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
			io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
			io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
			io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
			io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
			io.KeyMap[ImGuiKey_Home] = VK_HOME;
			io.KeyMap[ImGuiKey_End] = VK_END;
			io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
			io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
			io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
			io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
			io.KeyMap[ImGuiKey_A] = 'A';
			io.KeyMap[ImGuiKey_C] = 'C';
			io.KeyMap[ImGuiKey_V] = 'V';
			io.KeyMap[ImGuiKey_X] = 'X';
			io.KeyMap[ImGuiKey_Y] = 'Y';
			io.KeyMap[ImGuiKey_Z] = 'Z';

			io.ImeWindowHandle = window.GetHwnd();

			// Setup Dear ImGui style
			ImGui::StyleColorsDark();

			// Setup Platform/Renderer bindings
			ImGui_ImplWin32_Init(window.GetHwnd());
			ImGui_ImplDX11_Init(DX11::Device(), DX11::ImmediateContext());

			window.RegisterMessageCallback(WindowMessageCallback, nullptr);
		}

		void BeginFrame()
		{
			// Start the Dear ImGui frame
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		}

		void EndFrame()
		{
			// Rendering
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		void Shutdown()
		{
			ImGui_ImplDX11_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
		}
	} // namespace

App* GlobalApp = nullptr;


App::App(const wchar* appName, const wchar* iconResource) : 
	window(NULL, appName, WS_OVERLAPPEDWINDOW, WS_EX_APPWINDOW, 1280, 720, iconResource, iconResource),
    currentTimeDeltaSample(0),
    fps(0),
	applicationName(appName),
    createConsole(true),
	showWindow(true),
	returnCode(0)
{
    GlobalApp = this;
    for(uint32 i = 0; i < NumTimeDeltaSamples; ++i)
        timeDeltaBuffer[i] = 0;

}

int32 App::Run()
{
    try
    {
        window.SetClientArea(swapchain.Width(), swapchain.Height());
        if(showWindow) window.ShowWindow();
        window.RegisterMessageCallback(OnWindowResized, this);

        Initialize_private();
		
		Initialize(); // pure virtual

        while(window.IsAlive())
        {
            if(!window.IsMinimized())
            {
                timer.Update();

                CalculateFPS();

                BeginFrame_private();

                Update(timer); // pure virtual
                Render(timer); // pure virtual

                EndFrame_private();
            }
            window.MessageLoop();
        }

		Shutdown();
		Shutdown_private();
    }
	catch(SimpleFramework::Exception exception)
    {
        exception.ShowErrorMessage();
        return -1;
    }

    return returnCode;
}

void App::Initialize_private()
{
	DX11::Initialize(D3D_FEATURE_LEVEL_11_0);
    commandContext = new FDX11IGHIComputeCommandCotext;
	swapchain.Initialize(window);
	imgui::Initialize(window);
}

void App::BeginFrame_private()
{
	DX11::ImmediateContext()->OMSetRenderTargets(1, swapchain.RTV(), nullptr);
	DX11::ImmediateContext()->ClearRenderTargetView((swapchain.RTV())[0], clearColor);
	imgui::BeginFrame();
}

void App::EndFrame_private()
{
	imgui::EndFrame();
	swapchain.D3DSwapChain()->Present(0,0);
}

void App::Shutdown_private()
{
	imgui::Shutdown();
	swapchain.Shutdown();
	DX11::Shutdown();
}

App::~App()
{
}

void App::CalculateFPS()
{
}

void App::OnWindowResized(void* context, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    App* app = reinterpret_cast<App*>(context);
}

void App::Exit()
{
    window.Destroy();
}

void App::ToggleFullScreen(bool fullScreen)
{
}
ID3D11Device* App::Device()
{
	return DX11::Device();
}
ID3D11DeviceContext* App::ImmediateContext()
{
	return DX11::ImmediateContext();
}
uint32 App::SwapchainWidth() const
{
	return swapchain.Width();
}

uint32 App::SwapchainHeight() const
{
	return swapchain.Height();
}

std::string App::Name() const
{
	return WstringToStr(applicationName.c_str());
}

}