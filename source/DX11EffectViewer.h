#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#if D3D_COMPILER_VERSION < 46
#include <d3dx11.h>
#endif

#include <cstdio>

// code migration. https://msdn.microsoft.com/en-us/library/windows/desktop/ee418730(v=vs.85).aspx
// #include <xnamath.h> //has been replaced
#include <DirectXMath.h>

#include "imgui.h"
#include "EffectManager.h"
#include "Utils.h"
#include "App.h"

using namespace DirectX;

struct CB
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

class DX11EffectViewer : public SimpleFramework::App
{

public:
    DisplayMode mDisplayMode;
	LPCWSTR	    m_imageFilename;
	std::string m_imageName;

	DX11EffectViewer() 
		: App(L"Sample")
		, m_imageWidth(0)
		, m_imageHeight(0)
        //, m_imageFilename (L"../images/test.png")
        , m_imageName ("../images/test.png")
        , mDisplayMode(DisplayMode::SOURCE_RESULT )
	{

        static wchar_t wString[4096];
        MultiByteToWideChar(CP_ACP, 0, m_imageName.c_str(), -1, wString, 4096);
        m_imageFilename = wString;
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
		m_pd3dDevice = Device();
		m_pImmediateContext = ImmediateContext();
		initialize();
	}

    virtual void Shutdown() override;

	void SaveResult();
	
    void NextEffect(std::string &name)
    {
        ActiveEffect(EffectManager::GetEffectManager(m_pd3dDevice)->NextEffect(name));
    }

    void PrevEffect(std::string &name)
    {
        ActiveEffect(EffectManager::GetEffectManager(m_pd3dDevice)->PrevEffect(name));
    }

    void NextImage(std::string &name)
    {
        static wchar_t wString[4096];
        auto imageItr = mCurrentImage == mImageList.end() ? mCurrentImage = mImageList.begin() : mCurrentImage++;
        MultiByteToWideChar(CP_ACP, 0, (*imageItr).c_str(), -1, wString, 4096);
		m_imageName = (*imageItr);
        m_imageFilename = wString;
		Info("- Switch to image [%s]\n", m_imageName.c_str());

		//! All resource related need to rebuild 
		while (!LoadImageAsSrcTexture(m_pd3dDevice))
		{
			auto imageItr = mCurrentImage == mImageList.end() ? mCurrentImage = mImageList.begin() : mCurrentImage++;
			MultiByteToWideChar(CP_ACP, 0, (*imageItr).c_str(), -1, wString, 4096);
			m_imageName = (*imageItr);
			m_imageFilename = wString;
			Info("- Switch to image [%s]\n", m_imageName.c_str());
		}
        CreateCSInputTextureView(m_pd3dDevice);
        CreateCSOutputTextureAndView(m_pd3dDevice);
        CreateResultImageTextureAndView(m_pd3dDevice);
        UpdateCSConstBuffer();
        ActiveEffect(EffectManager::GetEffectManager(m_pd3dDevice)->NextEffect(name));
    }

    void    PrevImage(std::string &name)
    {
    }
    void    UpdateEffects()
    {
        EffectManager::GetEffectManager(m_pd3dDevice)->BuildEffects();
    }

    std::string    CurrentEffectName()
    {
        return EffectManager::GetEffectManager(m_pd3dDevice)->CurrentEffectName();
    }

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
    void    ActiveEffect(ID3D11ComputeShader* computeShader);
    void    UpdateCSConstBuffer();

	bool	InitGraphics(ID3D11Device* pd3dDevice);
	bool LoadImageAsSrcTexture(ID3D11Device* pd3dDevice);
    bool    CreateCSInputTextureView(ID3D11Device* pd3dDevice);
	bool CreateResultImageTextureAndView(ID3D11Device* pd3dDevice);
	bool	CreateCSInputTextureAndView(ID3D11Device* pd3dDevice);
	bool	CreateCSOutputTextureAndView(ID3D11Device* pd3dDevice);
	bool CreateCSConstBuffer(ID3D11Device* pd3dDevice);

	void    SetupViewport(float topLeftX, float topLeftY, int width, int height);

	void	RenderMultiViewport();
	void	RenderSourceImage();
	void	RenderResultImage();

	byte*	GetResultImage();
    
	// Fields
	int	m_imageWidth = 0;
	int	m_imageHeight = 0;
	double m_Aspect = 1.f;

	ID3D11Device*				m_pd3dDevice = nullptr;
	ID3D11DeviceContext*		m_pImmediateContext = nullptr;

	ID3D11VertexShader*			m_pVertexShader = nullptr;
	ID3D11PixelShader*			m_pPixelShaderSrcImage = nullptr;
	ID3D11PixelShader*			m_pPixelShaderResultImage = nullptr;
	ID3D11InputLayout*			m_pVertexLayout = nullptr;
	ID3D11Buffer*				m_pVertexBuffer = nullptr;
	ID3D11SamplerState*			m_pSamplerLinear = nullptr;

	UINT						m_textureSizeInBytes;

	ID3D11Texture2D*			m_srcImageTexture = nullptr;
	ID3D11ShaderResourceView*	m_srcImageTextureView = nullptr;
	ID3D11Texture2D*			m_resultImageTexture = nullptr;
	ID3D11ShaderResourceView*	m_resultImageTextureView = nullptr;

	ID3D11Buffer*               m_GPUConstBuffer = nullptr;
	ID3D11ComputeShader*		m_computeShader = nullptr;

    ID3D11Texture2D*            tempCSInputTexture = nullptr;
    ID3D11ShaderResourceView*   tempCSInputTextureView = nullptr;
    ID3D11Texture2D*            tempCSOutputTexture = nullptr;
    ID3D11UnorderedAccessView*  tempCSOutputTextureView = nullptr;

    // Used to copy result to CPU buffer
	ID3D11Texture2D *m_resultGPUCopy = nullptr;
	byte *m_resultCPUCopy = nullptr;

};