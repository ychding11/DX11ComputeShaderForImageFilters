#pragma once

// code migration. https://msdn.microsoft.com/en-us/library/windows/desktop/ee418730(v=vs.85).aspx
//#include <DirectXMath.h>

#include <vector> 
#include <unordered_map> 
#include <algorithm> 
#include "imgui.h"
#include "ImNodes.h"
#include "ImNodesEz.h"
#include "Filter.h"
#include "Utils.h"
#include "App.h"

#include "GHIResources.h"
#include "GHICommandContext.h"

#ifndef IMAGE_LOCATION
static_assert(0, "IMAGE_LOCATION is Not defined.");
#endif

#ifndef IMAGE_EFFECT_LOCATION
static_assert(0, "IMAGE_EFFECT_LOCATION is Not defined.");
#endif

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


class EffectViewer : public GHI::App
{

public:
    //DisplayMode mDisplayMode = DisplayMode::ONLY_SOURCE;
    DisplayMode mDisplayMode = DisplayMode::SOURCE_RESULT;
	std::string m_defaultImage;

	EffectViewer() 
		: App(L"Filters")
        , m_defaultImage(IMAGE_LOCATION + std::string("test.png"))
	{
		getFiles(IMAGE_EFFECT_LOCATION, mEffectList);
		getFiles(IMAGE_LOCATION, mImageList);
		mCurrentImage = mImageList.begin();
		mCurrentEffect = mEffectList.begin();

		window.RegisterMessageCallback(WindowMessageCallback,this);
	}

	void SaveResult();
	
    void NextEffect()
    { }

    void PrevEffect()
    { }

    void NextImage()
    { }

    void UpdateEffects()
    { }

protected:

	virtual void Initialize() override;

	virtual void Update(const GHI::Timer& timer) override
	{
		this->updateUI();
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
		mSrcTexture = mTextures[mImageList[mImageIndex]];
		mDstTexture = commandContext->CreateTextureByAnother(mSrcTexture);
		mFinalTexture = commandContext->CreateTextureByAnother(mSrcTexture);

		(*mCurFilter)->addInput(mSrcTexture);
		(*mCurFilter)->addOutput(mDstTexture);
		(*mCurFilter)->Active(commandContext);
		commandContext->CopyTexture(mFinalTexture, mDstTexture); //< dst <-- src
    }

	void updateUI();
	

	//bool CreateCSConstBuffer();
   // void UpdateCSConstBuffer();

    std::vector<std::string> mImageList;
    std::vector<std::string> mEffectList;
    std::vector<std::string>::iterator mCurrentImage;
    std::vector<std::string>::iterator mCurrentEffect;
	std::unordered_map<std::string, GHI::GHITexture *> mTextures;
	int mImageIndex = 0;
	int mEffectIndex = 0;
	int mActiveImageIndex = -1;
	int mActiveEffectIndex = -1;

	std::vector<Filter*> mFilters;
	std::vector<Filter*>::iterator mCurFilter;

	byte*	GetResultImage();
    
	GHI::GHIBuffer *mConstBuffer = nullptr;
	GHI::GHITexture *mSrcTexture = nullptr;
	GHI::GHITexture *mDstTexture = nullptr;
	GHI::GHITexture *mFinalTexture = nullptr;
};
