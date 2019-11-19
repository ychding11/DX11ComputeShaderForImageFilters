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
        updateUI();

        float elapsed = timer.ElapsedSecondsF();
        int size = Width < Height ? Width : Height;
        CB cb = { Width, Height, elapsed };
        commandContext->UpdateBuffer(mConstBuffer, &cb, sizeof(CB));
	}

	virtual void Render(const GHI::Timer& timer) override
	{
		if (compileShader)
		{
			compileShader = false;
			LoadShaderProgram("../data/canvas.hlsl");
		}

        if (curItem == 0)
            DrawCanvas((*shaderCache)["VSCanvas"], (*shaderCache)["PSCanvas"] );
        else if (curItem == 1)
            DrawCanvas((*shaderCache)["VSCanvas"], (*shaderCache)["PSClound"] );
        else if (curItem == 2)
            DrawCanvas((*shaderCache)["VSCanvas"], (*shaderCache)["PSfBM"] );
        //DrawCanvas((*shaderCache)["VSCanvas"], (*shaderCache)["PSSdfPrimitive"]);
        
	}

    virtual void Shutdown() override
    { }

	static void WindowMessageCallback(void* context, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    { }

private:

	void updateUI()
	{
        const char* items[] = { "Cavas", "Cloud", "fBM" };
        ImGui::Begin("settings");
        ImGui::Combo("Test", &curItem, items, IM_ARRAYSIZE(items));
        //ImGui::RadioButton("mytest", mytest );
		if (ImGui::Button("Compile"))
		{
			compileShader = true;
		}
        ImGui::End();
    }
    
	// Fields
	int	Width = 0;
	int	Height = 0;
    int curItem = 0;
    bool mytest;
	bool compileShader = true;

	GHI::GHIBuffer *mConstBuffer = nullptr;
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    Canvas viewer;
    viewer.Run();
    return 0;
}
