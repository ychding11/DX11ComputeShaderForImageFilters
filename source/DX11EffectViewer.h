#pragma once

// code migration. https://msdn.microsoft.com/en-us/library/windows/desktop/ee418730(v=vs.85).aspx
//#include <DirectXMath.h>

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
};

enum DisplayMode
{
    ONLY_SOURCE   = 0,
    ONLY_RESULT   = 1,
    SOURCE_RESULT = 2,
    ALL_MODE      = 3,
};

#define SHADERS_REPO "..\\effects"

class DX11EffectViewer : public GHI::App
{

public:
    //DisplayMode mDisplayMode = DisplayMode::ONLY_SOURCE;
    DisplayMode mDisplayMode = DisplayMode::SOURCE_RESULT;
	std::string m_defaultImage;

	DX11EffectViewer() 
		: App(L"Sample")
		, m_imageWidth(0)
		, m_imageHeight(0)
        , m_defaultImage("../images/test.png")
	{
		window.RegisterMessageCallback(WindowMessageCallback,this);
	}

	virtual void Initialize() override
	{
		initialize();
		std::vector<std::string> files;
		getFiles(SHADERS_REPO, files);
		shaderCache->InitComputeCache(files);

		shaderCache->EnumCache();
		mFilter = new BilaterialFilter();
		mFilter->Init(commandContext);
		mFilter->addInput(mSrcTexture);
		mFilter->addOutput(mDstTexture);
	}

	virtual void Update(const GHI::Timer& timer) override
	{
		this->updateUI();
		mFilter->UpdateUI(commandContext);
		mFilter->Active(commandContext);
		commandContext->CopyTexture(mFinalTexture, mDstTexture); //< dst <-- src
	}
	virtual void Render(const GHI::Timer& timer) override
	{
		Render();
	}


    virtual void Shutdown() override;

	void SaveResult();
	
    void NextEffect()
    {
        ActiveEffect(shaderCache->Current());
        shaderCache->Next();
    }

    void PrevEffect()
    {
        ActiveEffect(shaderCache->Current());
        shaderCache->Prev();
    }

    void NextImage()
    {
        mCurrentImage+1 == mImageList.end() ? mCurrentImage = mImageList.begin() : mCurrentImage++;
		DEBUG("Switch to image [%s]\n", (*mCurrentImage).c_str());

		while (!loadImage((*mCurrentImage)))
		{
			DEBUG("image [%s] load failed.\n", (*mCurrentImage).c_str());
			mCurrentImage+1 == mImageList.end() ? mCurrentImage = mImageList.begin() : mCurrentImage++;
			DEBUG("Switch to image [%s]\n", (*mCurrentImage).c_str());
		}

	    mDstTexture = commandContext->CreateTextureByAnother(mSrcTexture);
	    mFinalTexture = commandContext->CreateTextureByAnother(mSrcTexture);
        UpdateCSConstBuffer();
        ActiveEffect(shaderCache->Current());
    }

    void PrevImage()
    {
    }

    //! when shader code changes
    void UpdateEffects()
    { }

    int     imageHeight() const { return m_imageHeight; }
    int     imageWidth()  const { return m_imageWidth; }

protected:
		static void WindowMessageCallback(void* context, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:

	void	Render();
	int     initialize();

    std::vector<std::string> mImageList;
    std::vector<std::string>::iterator mCurrentImage;

    void    BuildImageList(const std::string &dir);
    void    ActiveEffect(GHI::GHIShader* computeShader);
    void    UpdateCSConstBuffer();

	bool InitGraphics();
	bool loadImage(std::string imagefile);
	bool CreateCSConstBuffer();

	void updateUI()
	{
		ImGui::Begin("UI");

		ImGui::Text("This is experialment."); // Display some text (you can use a format strings too)
		ImGui::ColorEdit4("clear color", clearColor, ImGuiColorEditFlags_Float); // floats representing a color

		if (ImGui::Button("Save")) // Buttons return true when clicked (most widgets return true when edited/activated)
		{
		}
		ImGui::SameLine();
		ImGui::Text("Application Average: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();

		static ImNodes::CanvasState canvas;

		if (ImGui::Begin("ImNodes", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			ImNodes::BeginCanvas(&canvas);

			struct Node
			{
				ImVec2 pos{};
				bool selected{};
				ImNodes::Ez::SlotInfo inputs[1];
				ImNodes::Ez::SlotInfo outputs[1];
			};

			static Node nodes[3] = {
				{{50, 100}, false, {{"In", 1}}, {{"Out", 1}}},
				{{250, 50}, false, {{"In", 1}}, {{"Out", 1}}},
				{{250, 100}, false, {{"In", 1}}, {{"Out", 1}}},
			};

			for (Node& node : nodes)
			{
				if (ImNodes::Ez::BeginNode(&node, "Node Title", &node.pos, &node.selected))
				{
					ImNodes::Ez::InputSlots(node.inputs, 1);
					ImNodes::Ez::OutputSlots(node.outputs, 1);
					ImNodes::Ez::EndNode();
				}
			}

			ImNodes::Connection(&nodes[1], "In", &nodes[0], "Out");
			ImNodes::Connection(&nodes[2], "In", &nodes[1], "Out");

			ImNodes::EndCanvas();
		}
		ImGui::End();
	}

	void	RenderMultiViewport();
	void	RenderSourceImage();
	void	RenderResultImage();

	byte*	GetResultImage();
    
	// Fields
	int	m_imageWidth = 0;
	int	m_imageHeight = 0;
	float m_Aspect = 1.f;
	UINT m_textureSizeInBytes;

	GHI::GHIBuffer *mConstBuffer = nullptr;
	GHI::GHITexture *mSrcTexture = nullptr;
	GHI::GHITexture *mDstTexture = nullptr;
	GHI::GHITexture *mFinalTexture = nullptr;
	Filter *mFilter = nullptr;
};