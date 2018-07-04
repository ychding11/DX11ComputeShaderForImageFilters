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

using namespace DirectX;

// Constant Buffer Layout
struct CB
{
	UINT iWidth;
	UINT iHeight;
};

class DXApplication
{
public:
	DXApplication() 
		: m_pd3dDevice(NULL)
		, m_pImmediateContext(NULL)
		, m_pSwapChain(NULL)
		, m_pRenderTargetView(NULL)
		, m_srcImageTexture(NULL)
		, m_srcTextureData(NULL)
		, m_destTexture(NULL)
		, m_computeShader(NULL)
		, m_pVertexLayout(NULL)
		, m_GPUConstBuffer(NULL)
		, m_dstDataBufferGPUCopy(NULL)
		, m_dstDataBufferCPUCopy(NULL)
		, m_imageWidth(0)
		, m_imageHeight(0)
	{}

	bool	initialize(HWND hwnd, int w, int h);
	bool	RunComputeShader( LPCWSTR shaderFilename );
	void	runGaussianFilter( LPCWSTR shaderFilename );
    void	update() {}
	void	RenderResult();
	void	release();
    int     imageHeight() const { return m_imageHeight; }
    int     imageWidth()  const { return m_imageWidth; }

private:
	void	InitGraphics();
	void	releaseFullScreenQuad() {}
	void	LoadImageAsTexture(LPCWSTR filename, ID3D11Texture2D** texture);
	void    CreateResultImageTextureAndView();
	void    CreateCSConstBuffer();
	void    SetupViewport(int width, int height);
	void	CreateCSInputTextureAndView();
	void	CreateCSOutputTextureAndView();
    bool	LoadComputeShader(LPCWSTR filename, LPCSTR entrypoint, ID3D11ComputeShader** computeShader);
	byte*	getCPUCopyOfGPUDestBuffer();

	// Fields
	int							m_imageWidth;
	int							m_imageHeight;

	ID3D11Device*				m_pd3dDevice;
	ID3D11DeviceContext*		m_pImmediateContext;
	IDXGISwapChain*				m_pSwapChain;
	ID3D11RenderTargetView*		m_pRenderTargetView;

	ID3D11VertexShader*			m_pVertexShader;
	ID3D11PixelShader*			m_pPixelShader;
	ID3D11InputLayout*			m_pVertexLayout;
	ID3D11Buffer*				m_pVertexBuffer;
	ID3D11SamplerState*			m_pSamplerLinear;

	UINT						m_textureDataSize;

	ID3D11Texture2D*			m_srcImageTexture;
	ID3D11ShaderResourceView*	m_srcImageTextureView;
	byte*						m_srcTextureData;

	ID3D11Texture2D*			m_destTexture;
	ID3D11ShaderResourceView*	m_resultImageTextureView;
	ID3D11Buffer*               m_GPUConstBuffer;
	ID3D11ComputeShader*		m_computeShader;
	ID3D11Buffer*				m_dstDataBufferGPUCopy;
	byte*						m_dstDataBufferCPUCopy;
    ID3D11Texture2D*            tempCSInputTexture;
    ID3D11ShaderResourceView*   tempCSInputTextureView;
    ID3D11Texture2D*            tempCSOutputTexture;
    ID3D11UnorderedAccessView*   tempCSOutputTextureView;
};