#pragma once

// code migration. https://msdn.microsoft.com/en-us/library/windows/desktop/ee418730(v=vs.85).aspx
//#include <DirectXMath.h>

#include "imgui.h"
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

class DX11EffectViewer : public SimpleFramework::App
{

public:
    //DisplayMode mDisplayMode = DisplayMode::ONLY_SOURCE;
    DisplayMode mDisplayMode = DisplayMode::SOURCE_RESULT;
	std::string m_imageName;

	DX11EffectViewer() 
		: App(L"Sample")
		, m_imageWidth(0)
		, m_imageHeight(0)
        , m_imageName ("../images/test.png")
	{
		window.RegisterMessageCallback(WindowMessageCallback,this);
	}

	virtual void Update(const SimpleFramework::Timer& timer) override
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
	virtual void Render(const SimpleFramework::Timer& timer) override
	{
		Render();
	}


	virtual void Initialize() override
	{
		initialize();
		std::vector<std::string> files;
		getFiles(SHADERS_REPO, files);
		shaderCache->InitComputeCache(files);

		shaderCache->EnumCache();
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
        mCurrentImage == mImageList.end() ? mCurrentImage = mImageList.begin() : mCurrentImage++;
		m_imageName = (*mCurrentImage);
		INFO("Switch to image [%s]\n", m_imageName.c_str());

		while (!loadImage())
		{
			mCurrentImage == mImageList.end() ? mCurrentImage = mImageList.begin() : mCurrentImage++;
			m_imageName = (*mCurrentImage);
			INFO("Switch to image [%s]\n", m_imageName.c_str());
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
    void    ActiveEffect(SimpleFramework::GHIShader* computeShader);
    void    UpdateCSConstBuffer();

	bool InitGraphics();
	bool loadImage();
	bool CreateCSConstBuffer();

	void	RenderMultiViewport();
	void	RenderSourceImage();
	void	RenderResultImage();

	byte*	GetResultImage();
    
	// Fields
	int	m_imageWidth = 0;
	int	m_imageHeight = 0;
	float m_Aspect = 1.f;
	UINT m_textureSizeInBytes;

	SimpleFramework::GHIBuffer *mConstBuffer = nullptr;
	SimpleFramework::GHITexture *mSrcTexture = nullptr;
	SimpleFramework::GHITexture *mDstTexture = nullptr;
	SimpleFramework::GHITexture *mFinalTexture = nullptr;
};