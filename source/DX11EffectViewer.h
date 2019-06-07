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

#include "EffectManager.h"
#include "Utils.h"

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

extern unsigned int widthSwapchain;
extern unsigned int heightSwapchain;
class DX11EffectViewer
{

	struct SimpleVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Tex;
	};

public:
    DisplayMode mDisplayMode;
	LPCWSTR	    m_imageFilename;
	std::string m_imageName;

	DX11EffectViewer() 
		: m_pd3dDevice(NULL)
		, m_pImmediateContext(NULL)
		, m_srcImageTexture(NULL)
		, m_resultImageTexture(NULL)
		, m_computeShader(NULL)
		, m_pVertexLayout(NULL)
		, m_GPUConstBuffer(NULL)
		, m_resultGPUCopy(NULL)
		, m_resultCPUCopy(NULL)
		, m_imageWidth(0)
		, m_imageHeight(0)
        //, m_imageFilename (L"../images/test.png")
        , m_imageName ("../images/test.png")
        , mDisplayMode(DisplayMode::SOURCE_RESULT )
	{

        static wchar_t wString[4096];
        MultiByteToWideChar(CP_ACP, 0, m_imageName.c_str(), -1, wString, 4096);
        m_imageFilename = wString;
	}

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

	void	Render(ID3D11DeviceContext* pImmediateContext );
	void	Destory();

	int     initialize(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext);

    int     imageHeight() const { return m_imageHeight; }
    int     imageWidth()  const { return m_imageWidth; }

private:

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

	byte*	getCPUCopyOfGPUDestBuffer();
    
	// Fields
	int	m_imageWidth;
	int	m_imageHeight;
	double m_Aspect;

	ID3D11Device*				m_pd3dDevice;
	ID3D11DeviceContext*		m_pImmediateContext;

	ID3D11VertexShader*			m_pVertexShader;
	ID3D11PixelShader*			m_pPixelShaderSrcImage;
	ID3D11PixelShader*			m_pPixelShaderResultImage;
	ID3D11InputLayout*			m_pVertexLayout;
	ID3D11Buffer*				m_pVertexBuffer;
	ID3D11SamplerState*			m_pSamplerLinear;

	UINT						m_textureSizeInBytes;

	ID3D11Texture2D*			m_srcImageTexture;
	ID3D11ShaderResourceView*	m_srcImageTextureView;
	ID3D11Texture2D*			m_resultImageTexture;
	ID3D11ShaderResourceView*	m_resultImageTextureView;

	ID3D11Buffer*               m_GPUConstBuffer;
	ID3D11ComputeShader*		m_computeShader;

    ID3D11Texture2D*            tempCSInputTexture;
    ID3D11ShaderResourceView*   tempCSInputTextureView;
    ID3D11Texture2D*            tempCSOutputTexture;
    ID3D11UnorderedAccessView*  tempCSOutputTextureView;

    // Used to copy result to CPU buffer
	ID3D11Texture2D *m_resultGPUCopy;
	byte *m_resultCPUCopy;

};