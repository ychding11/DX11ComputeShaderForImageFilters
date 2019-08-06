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
		: App(L"Filters")
		, m_imageWidth(0)
		, m_imageHeight(0)
        , m_defaultImage("../images/test.png")
	{
		window.RegisterMessageCallback(WindowMessageCallback,this);
	}

	void SaveResult();
	
    void NextEffect()
    {
		mCurFilter+1 == mFilters.end() ? mCurFilter = mFilters.begin() : mCurFilter++;
        activeCurFilter();
    }

    void PrevEffect()
    {
		mCurFilter == mFilters.begin() ? mCurFilter = mFilters.end()-1 : mCurFilter--;
        activeCurFilter();
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

        activeCurFilter();
    }

    void UpdateEffects()
    { }

    int     imageHeight() const { return m_imageHeight; }
    int     imageWidth()  const { return m_imageWidth; }

protected:

	virtual void Initialize() override
	{
		initialize();
		std::vector<std::string> files;
		getFiles(SHADERS_REPO, files);

		Filter *filter = nullptr;

#if 0
		for (auto it = files.begin(); it != files.end() && (*it) != "..\\effects\\test.hlsl"; ++it)
		{
			filter = new Filter(*it);
			filter->Init(commandContext);
			filter->setSampler(linearSampler);
			mFilters.push_back(filter);
		}
#endif

		filter = new BilaterialFilter();
		filter->Init(commandContext);
		filter->setSampler(linearSampler);
		mFilters.push_back(filter);

		filter = new FishEyeFilter();
		filter->Init(commandContext);
		filter->setSampler(linearSampler);
		mFilters.push_back(filter);

		filter = new SwirlFilter();
		filter->Init(commandContext);
		filter->setSampler(linearSampler);
		mFilters.push_back(filter);

		filter = new LensCircleFilter();
		filter->Init(commandContext);
		filter->setSampler(linearSampler);
		mFilters.push_back(filter);

		mCurFilter = mFilters.begin();
        activeCurFilter();
	}

	virtual void Update(const GHI::Timer& timer) override
	{
		this->updateUI();
		(*mCurFilter)->UpdateUI(commandContext);
	}
	virtual void Render(const GHI::Timer& timer) override
	{
		render();
	}

    virtual void Shutdown() override;

	static void WindowMessageCallback(void* context, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:

	void	render();
	int     initialize();

    void activeCurFilter()
    {
		(*mCurFilter)->addInput(mSrcTexture);
		(*mCurFilter)->addOutput(mDstTexture);
		(*mCurFilter)->Active(commandContext);
		commandContext->CopyTexture(mFinalTexture, mDstTexture); //< dst <-- src
    }

    std::vector<std::string> mImageList;
    std::vector<std::string>::iterator mCurrentImage;

	bool loadImage(std::string imagefile);
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
	}

	//bool CreateCSConstBuffer();
   // void UpdateCSConstBuffer();

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
	std::vector<Filter*> mFilters;
	std::vector<Filter*>::iterator mCurFilter;
};