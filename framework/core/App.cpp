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

namespace GHI
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

	extern ShaderCache *gShaderCache;

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
		catch(GHI::Exception exception)
		{
			exception.ShowErrorMessage();
			return -1;
		}

		return returnCode;
	}

    /* Create device, command context, swapchain and other device dependent resources */
	void App::Initialize_private()
	{
		DX11::Initialize(D3D_FEATURE_LEVEL_11_0);
		commandContext = new FDX11IGHIComputeCommandCotext;
		gShaderCache = new ShaderCache(commandContext);
		//shaderCache = new ShaderCache(commandContext);
		shaderCache = gShaderCache;

		GHISamplerDesc desc;
		linearSampler = (commandContext->CreateSampler(desc));
		swapchain.Initialize(window);
		imgui::Initialize(window);

		LoadShaderProgram("../data/fullQuad.fx");
	}

    /* reclaim all tracked resources */
	void App::Shutdown_private()
	{
		for (auto it = GHIResource::list.begin(); it != GHIResource::list.end(); ++it)
		{
			(*it)->release();
		}
		imgui::Shutdown();
		swapchain.Shutdown();
		DX11::Shutdown();
	}


	void App::BeginFrame_private()
	{
		DX11::ImmediateContext()->OMSetRenderTargets(1, swapchain.RTV(), nullptr);
		DX11::ImmediateContext()->ClearRenderTargetView((swapchain.RTV())[0], clearColor);
        commandContext->SetViewport({0., 0., (float)SwapchainWidth(), (float)SwapchainHeight(), 0., 1.});
		imgui::BeginFrame();
	}

	void App::EndFrame_private()
	{
		imgui::EndFrame();
		swapchain.D3DSwapChain()->Present(0,0);
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
		return ToStr(applicationName);
	}

    void App::DrawFullScreenTriangle(GHIViewport viewport, GHITexture *tex)
    {
        commandContext->SetShader((*shaderCache)["VS"]);
		commandContext->setPrimitiveTopology(PrimitiveTopology::TOPOLOGY_TRIANGLESTRIP);
		commandContext->SetViewport(viewport);
        commandContext->SetShaderResource(tex, 0, GHISRVParam(), EShaderStage::PS);
        commandContext->SetSampler(linearSampler, 0, EShaderStage::PS);
        commandContext->SetShader((*shaderCache)["PS"]);
		commandContext->Draw(3, 0);
    }

	void App::LoadShaderProgram(std::string file)
	{
		std::ifstream in(file);
		std::string str;
		while (std::getline(in, str))
		{
			if (str.size() <= 0)
				break;
			std::vector<std::string> symbols = Split(str);
			if (symbols[1] == "VS")
			{
				GHIVertexShader* vs = commandContext->CreateVertexShader(file, symbols[2]);
				shaderCache->AddGraphicShader(symbols[2], vs);
			}
			else if (symbols[1] == "PS")
			{
				GHIPixelShader* ps = commandContext->CreatePixelShader(file, symbols[2]);
				shaderCache->AddGraphicShader(symbols[2], ps);
			}
		}
		in.close();
	}


    ////==========================================////
    //// Input handle 
    ////==========================================////
    MouseState MouseState::prevState;
    KeyboardState KeyboardState::prevState;
    BYTE KeyboardState::currState[256];

    MouseState::MouseState() :
        X(0),
        Y(0),
        DX(0),
        DY(0),
        IsOverWindow(false)
    { }

    // platform specific
    MouseState MouseState::GetMouseState(HWND hwnd)
    {
        POINT pos;
        if (!GetCursorPos(&pos))
            throw Win32Exception(GetLastError());

        if (hwnd != NULL)
            if (!ScreenToClient(hwnd, &pos))
                throw Win32Exception(GetLastError());

        MouseState newState;
        newState.X = pos.x;
        newState.Y = pos.y;
        newState.DX = pos.x - prevState.X;
        newState.DY = pos.y - prevState.Y;

        newState.LButton.Pressed = (GetKeyState(VK_LBUTTON) & 0x8000) > 0;
        newState.LButton.RisingEdge = newState.LButton.Pressed && !prevState.LButton.Pressed;
        newState.LButton.RisingEdge = !newState.LButton.Pressed && prevState.LButton.Pressed;

        newState.MButton.Pressed = (GetKeyState(VK_MBUTTON) & 0x8000) > 0;
        newState.MButton.RisingEdge = newState.MButton.Pressed && !prevState.MButton.Pressed;
        newState.MButton.RisingEdge = !newState.MButton.Pressed && prevState.MButton.Pressed;

        newState.RButton.Pressed = (GetKeyState(VK_RBUTTON) & 0x8000) > 0;
        newState.RButton.RisingEdge = newState.RButton.Pressed && !prevState.RButton.Pressed;
        newState.RButton.RisingEdge = !newState.RButton.Pressed && prevState.RButton.Pressed;

        if (hwnd != NULL)
        {
            RECT clientRect;
            if (!::GetClientRect(hwnd, &clientRect))
                throw Win32Exception(GetLastError());
            newState.IsOverWindow = (pos.x >= 0 && pos.x < clientRect.right && pos.y >= 0 && pos.y < clientRect.bottom);
        }
        else
            newState.IsOverWindow = false;

        prevState = newState;
        return prevState;
    }

    // platform specific
    void MouseState::SetCursorPos(int x, int y, HWND hwnd)
    {
        POINT pos;
        pos.x = x;
        pos.y = y;

        if (hwnd != NULL)
            if (!ClientToScreen(hwnd, &pos))
                throw Win32Exception(GetLastError());

        if (!::SetCursorPos(pos.x, pos.y))
            throw Win32Exception(GetLastError());
    }

    KeyState KeyboardState::GetKeyState(Keys key) const
    {
        return keyStates[key];
    }

    bool KeyboardState::IsKeyDown(Keys key) const
    {
        return keyStates[key].Pressed;
    }

    bool KeyboardState::RisingEdge(Keys key) const
    {
        return keyStates[key].RisingEdge;
    }

    // Platfrom specific
    KeyboardState KeyboardState::GetKeyboardState(HWND hwnd)
    {
        if (GetForegroundWindow() != hwnd)
            return prevState;

        ::GetKeyboardState(currState);

        KeyState state;
        for (UINT i = 0; i < 256; ++i)
        {
            state.Pressed = KeyPressed(currState[i]);
            state.RisingEdge = state.Pressed && !prevState.keyStates[i].Pressed;
            state.FallingEdge = !state.Pressed && prevState.keyStates[i].Pressed;
            prevState.keyStates[i] = state;
        }

        return prevState;
    }

    KeyState::KeyState() :
        Pressed(false),
        RisingEdge(false),
        FallingEdge(false)
    { }

}