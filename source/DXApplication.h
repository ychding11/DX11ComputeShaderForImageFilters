#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#if D3D_COMPILER_VERSION < 46
#include <d3dx11.h>
#endif

#include <cstdio>

// code migration. https://msdn.microsoft.com/en-us/library/windows/desktop/ee418730(v=vs.85).aspx
//#include <xnamath.h> //has been replaced
#include <DirectXMath.h>

using namespace DirectX;

// Constant Buffer Layout
struct CB
{
	UINT iWidth;
	UINT iHeight;
};

/**
*	This class contains all the system stuff that we need to render with OpenGL
*/
class DXApplication {
public:
	// Ctor
	DXApplication() 
		: m_pd3dDevice(NULL)
		, m_pImmediateContext(NULL)
		, m_pSwapChain(NULL)
		, m_pRenderTargetView(NULL)
		, m_srcTexture(NULL)
		, m_srcTextureData(NULL)
		, m_srcDataGPUBuffer(NULL)
		, m_srcDataGPUBufferView(NULL)
		, m_destTexture(NULL)
		, m_destDataGPUBuffer(NULL)
		, m_destDataGPUBufferView(NULL)
		, m_computeShader(NULL)
		, m_pVertexLayout(NULL)
		, g_pConstBuffer(NULL)
		, m_imageWidth(0)
		, m_imageHeight(0)
	{}

	// Methods
	bool	initialize(HWND hwnd, int w, int h);
	bool	runComputeShader( LPCWSTR shaderFilename );
	void	update();
	void	render();
	void	release();

	// we prefer it as an inline implementation
	void    updateShaderConst(UINT iWidth, UINT iHeight)
	{
		CB cb = { iWidth, iHeight };
		//m_pImmediateContext->UpdateSubresource(g_pConstBuffer, 0, nullptr, &cb, 0, 0);
		m_pImmediateContext->CSSetConstantBuffers(0, 1, &g_pConstBuffer);
		printf("- Update const buffer %d %d.\n", iWidth, iHeight);
	}

private:
	// Methods
	bool	loadFullScreenQuad();
	void	releaseFullScreenQuad() {}
	bool	loadTextureAndCheckFomart(LPCWSTR filename, ID3D11Texture2D** texture);
	bool	copyTexture();
	bool	createInputBuffer();
	bool	createOutputBuffer();
	bool	loadComputeShader( LPCWSTR filename, ID3D11ComputeShader** computeShader);
	byte*	getCPUCopyOfGPUDestBuffer();

	// Fields
	int							m_windowWidth;
	int							m_windowHeight;
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

	ID3D11Texture2D*			m_srcTexture;
	ID3D11ShaderResourceView*	m_srcTextureView;
	byte*						m_srcTextureData;
	ID3D11Buffer*				m_srcDataGPUBuffer;
	ID3D11ShaderResourceView*	m_srcDataGPUBufferView;

	ID3D11Texture2D*			m_destTexture;
	ID3D11ShaderResourceView*	m_destTextureView;
	ID3D11Buffer*				m_destDataGPUBuffer;
	ID3D11UnorderedAccessView*	m_destDataGPUBufferView;
	ID3D11Buffer*               g_pConstBuffer;
	ID3D11ComputeShader*		m_computeShader;
};