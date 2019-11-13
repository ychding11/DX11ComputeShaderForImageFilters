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
    float fFrequency;
    float fAltitude;
};



class Fractal: public GHI::App
{

public:

	Fractal() : App(L"Fractal")
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
        CB cb = { Width, Height, elapsed, frequency, altitude  };
        commandContext->UpdateBuffer(mConstBuffer, &cb, sizeof(CB));
	}

	virtual void Render(const GHI::Timer& timer) override
	{
        //GHI::GHIViewport viewport = {0, 0, Height, Height, 0, 1};

		if (compileShader)
		{
			compileShader = false;
			LoadShaderProgram("../data/fractal.hlsl");
		}

        //if (curItem == 0)
            DrawCanvas((*shaderCache)["VSCanvas"], (*shaderCache)["PSMandelbrot"] );
	}

    virtual void Shutdown() override
    { }

	static void WindowMessageCallback(void* context, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    { }

private:

	void updateUI()
	{
        const char* items[] = { "Mandelbrot" };
        ImGui::Begin("settings");
        ImGui::Combo("Test", &curItem, items, IM_ARRAYSIZE(items));
        //ImGui::RadioButton("mytest", mytest );
		if (ImGui::Button("Compile"))
		{
			compileShader = true;
		}
        ImGui::SliderFloat("Frequency", &frequency,  0.2, 0.8);
        ImGui::SliderFloat("Altitude", &altitude,  1, 10);
        ImGui::End();
    }
    
	// Fields
	int	Width = 0;
	int	Height = 0;
    int curItem = 0;
    float frequency = 0.23;
    float altitude = 3.0;
	bool compileShader = true;

	GHI::GHIBuffer *mConstBuffer = nullptr;
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    Fractal viewer;
    viewer.Run();
    return 0;
}
