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
	int iAnimateSphere;
	int iAnimateLight;
	float fLightColor[3];
};



class Canvas: public GHI::App
{

public:

	Canvas() : App(L"Sphere")
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
        swapchain->SetPresentMode(GHI::EPresentMode::ThreeVerticalInterval);

        int size = Width < Height ? Width : Height;

        CB cb = { size, size};
        mConstBuffer = commandContext->CreateConstBuffer(sizeof(cb), &cb);
        commandContext->SetConstBuffer(mConstBuffer, 0, (*shaderCache)["PSSphere"]);
	}

	virtual void Update(const GHI::Timer& timer) override
	{
		updateUI();
        float elapsed = timer.ElapsedMicrosecondsF();
		CB cb = { Width, Height, elapsed, animateSphere == true ? 1 : 0, animateLight == true ? 1 : 0, {lightColor[0],lightColor[1],lightColor[2]} };
        commandContext->UpdateBuffer(mConstBuffer, &cb, sizeof(CB));
	}

	virtual void Render(const GHI::Timer& timer) override
	{
        //GHI::GHIViewport viewport = {0, 0, Height, Height, 0, 1};
        DrawCanvas((*shaderCache)["VSSphere"], (*shaderCache)["PSSphere"] );
	}

    virtual void Shutdown() override
    { }

	static void WindowMessageCallback(void* context, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    { }

private:

	void updateUI()
	{
		ImGui::Begin("settings");
		ImGui::Checkbox("animate sphere", &animateSphere);
		ImGui::Checkbox("animate light", &animateLight);
		ImGui::ColorEdit3("light color", lightColor);
		ImGui::End();
	}
    
	// Fields
	int	Width = 0;
	int	Height = 0;
	bool animateSphere = 1;
	bool animateLight = 1;
	float lightColor[3] = {1., 1, 1};

	GHI::GHIBuffer *mConstBuffer = nullptr;
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    Canvas viewer;
    viewer.Run();
    return 0;
}
