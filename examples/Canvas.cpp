#pragma once

// code migration. https://msdn.microsoft.com/en-us/library/windows/desktop/ee418730(v=vs.85).aspx
//#include <DirectXMath.h>

#include <vector> 
#include "imgui.h"
#include "ImNodes.h"
#include "ImNodesEz.h"
#include "Filter.h"
#include "Utils.h"
#include "App.h"

#include "GHIResources.h"
#include "GHICommandContext.h"

struct alignas(16) CB
{
	int iWidth;
	int iHeight;
    float fTime;
};



class Canvas: public GHI::App
{

public:

	Canvas() : App(L"Canvas")
	{
		window.RegisterMessageCallback(WindowMessageCallback,this);
	}

	
    void NextEffect()
    {
    }

    void PrevEffect()
    {
    }


    void UpdateEffects()
    { }

protected:

	virtual void Initialize() override
	{
        Width = SwapchainWidth();
        Height = SwapchainHeight();

        int size = Width < Height ? Width : Height;

        CB cb = { size, size};
        mConstBuffer = commandContext->CreateConstBuffer(sizeof(cb), &cb);
        commandContext->SetConstBuffer(mConstBuffer, 0, (*shaderCache)["PSCanvas"]);
	}

	virtual void Update(const GHI::Timer& timer) override
	{
        float elapsed = timer.ElapsedSecondsF();
        int size = Width < Height ? Width : Height;
        //CB cb = { size, size, elapsed };
        CB cb = { Width, Height, elapsed };
        commandContext->UpdateBuffer(mConstBuffer, &cb, sizeof(CB));
	}

	virtual void Render(const GHI::Timer& timer) override
	{
        //GHI::GHIViewport viewport = {0, 0, Height, Height, 0, 1};
        DrawCanvas((*shaderCache)["VSCanvas"], (*shaderCache)["PSCanvas"] );
        //DrawCanvas((*shaderCache)["VSCanvas"], (*shaderCache)["PSSdfPrimitive"]);
        
	}

    virtual void Shutdown() override
    { }

	static void WindowMessageCallback(void* context, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    { }

private:

	void updateUI()
	{ }
    
	// Fields
	int	Width = 0;
	int	Height = 0;

	GHI::GHIBuffer *mConstBuffer = nullptr;
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    Canvas viewer;
    viewer.Run();
    return 0;
}
