#include "DX11EffectViewer.h"
#include "WICTextureLoader.h"
#include "EffectManager.h"
#include "Utils.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Safe Release Function
template <class T>
void SafeRelease(T **ppT)
{
	if (ppT && *ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

void   DX11EffectViewer::BuildImageList(const std::string &dir)
{
    getFiles(dir, mImageList);
    mCurrentImage = mImageList.begin();
}

int	DX11EffectViewer::initialize()
{
	InitGraphics(m_pd3dDevice);

	LoadImageAsSrcTexture(m_pd3dDevice); //< Load source image as texture and upate image size.
    CreateResultImageTextureAndView(m_pd3dDevice);
	CreateCSConstBuffer(m_pd3dDevice);
	CreateCSInputTextureAndView(m_pd3dDevice);
	CreateCSOutputTextureAndView(m_pd3dDevice);

    BuildImageList(IMAGE_REPO);

    EffectManager::GetEffectManager(m_pd3dDevice)->CheckEffect();
	Info("DX11EffectViewer Initialized OK. image [%s]\n", m_imageName.c_str());
	return 0;
}

void  DX11EffectViewer::ActiveEffect(ID3D11ComputeShader* computeShader)
{
	ID3D11UnorderedAccessView *ppUAViewNULL[2] = { NULL, NULL };
	ID3D11ShaderResourceView  *ppSRVNULL[2]    = { NULL, NULL };

    if (computeShader == NULL) return;
	m_pImmediateContext->CSSetShader( computeShader, NULL, 0 );
    m_pImmediateContext->CSSetShaderResources(0, 1, &tempCSInputTextureView);
    m_pImmediateContext->CSSetUnorderedAccessViews(0, 1, &tempCSOutputTextureView, NULL);
	m_pImmediateContext->CSSetConstantBuffers(0, 1, &m_GPUConstBuffer);
    m_pImmediateContext->CSSetSamplers( 0, 1, &m_pSamplerLinear );

	// Dispatch returns immediately ?
	m_pImmediateContext->Dispatch( (m_imageWidth + 31) / 32, (m_imageHeight + 31) / 32, 1 );

	m_pImmediateContext->CSSetShader( NULL, NULL, 0 );
	m_pImmediateContext->CSSetUnorderedAccessViews( 0, 1, ppUAViewNULL, NULL );
	m_pImmediateContext->CSSetShaderResources( 0, 1, ppSRVNULL );

    m_pImmediateContext->CopyResource(m_resultImageTexture, tempCSOutputTexture); //< dst <-- src
}

void DX11EffectViewer::Render() 
{
	m_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
	m_pImmediateContext->VSSetShader( m_pVertexShader, NULL, 0 );
	m_pImmediateContext->PSSetSamplers( 0, 1, &m_pSamplerLinear );

    if (mDisplayMode == DisplayMode::ONLY_RESULT)
    {
        RenderResultImage();
    }
    else if (mDisplayMode == DisplayMode::ONLY_SOURCE)
    {
        RenderSourceImage();
    }
    else if (mDisplayMode == DisplayMode::SOURCE_RESULT)
    {
        RenderMultiViewport();
    }
    else
    {
        m_pImmediateContext->PSSetShaderResources(0, 1, &m_srcImageTextureView );
        m_pImmediateContext->PSSetShader( m_pPixelShaderSrcImage, NULL, 0 );
        SetupViewport(0.f, 0.f, m_imageWidth, m_imageHeight);
        m_pImmediateContext->Draw( 3, 0 );// draw non-indexed non-instanced primitives.[vertex count, vertex offset in vertex buffer]

        m_pImmediateContext->PSSetShaderResources(1, 1, &m_resultImageTextureView );
        m_pImmediateContext->PSSetShader( m_pPixelShaderResultImage, NULL, 0 );
        SetupViewport(m_imageWidth + 1.f, 0.f, m_imageWidth, m_imageHeight);
        m_pImmediateContext->Draw( 3, 0 );// draw non-indexed non-instanced primitives.[vertex count, vertex offset in vertex buffer]
    }
}

void DX11EffectViewer::RenderMultiViewport()
{
	int width, height;
	width = float(SwapchainWidth() - 2) / 2.;
	height = 1. / m_Aspect * width;
	m_pImmediateContext->PSSetShaderResources( 0, 1, &m_srcImageTextureView );
	m_pImmediateContext->PSSetShader( m_pPixelShaderSrcImage, NULL, 0 );
	SetupViewport(0.f, 0.f, width, height);
	m_pImmediateContext->Draw( 3, 0 );

	m_pImmediateContext->PSSetShaderResources( 1, 1, &m_resultImageTextureView );
	m_pImmediateContext->PSSetShader( m_pPixelShaderResultImage, NULL, 0 );
	SetupViewport(width + 2.f, 0.f, width, height);
	m_pImmediateContext->Draw( 3, 0 );
}

void	DX11EffectViewer::RenderSourceImage()
{
	m_pImmediateContext->PSSetShaderResources( 0, 1, &m_srcImageTextureView );
	m_pImmediateContext->PSSetShader( m_pPixelShaderSrcImage, NULL, 0 );
	SetupViewport(0.f, 0.f, m_imageWidth, m_imageHeight);
	m_pImmediateContext->Draw( 3, 0 );
}

void	DX11EffectViewer::RenderResultImage()
{
	m_pImmediateContext->PSSetShaderResources( 1, 1, &m_resultImageTextureView );
	m_pImmediateContext->PSSetShader( m_pPixelShaderResultImage, NULL, 0 );
	SetupViewport(0.f, 0.f, m_imageWidth, m_imageHeight);
	m_pImmediateContext->Draw( 3, 0 );
}

void DX11EffectViewer::Shutdown() 
{
	SafeRelease(&m_computeShader);

	SafeRelease(&m_resultImageTexture);
	SafeRelease(&m_resultImageTextureView);

	SafeRelease(&m_srcImageTexture);
	SafeRelease(&m_srcImageTextureView);

	SafeRelease(&m_pVertexBuffer);
	SafeRelease(&m_pVertexLayout);
	SafeRelease(&m_pVertexShader);
	SafeRelease(&m_pPixelShaderSrcImage);

	SafeRelease(&m_pSamplerLinear);

	if( m_pImmediateContext ) m_pImmediateContext->ClearState();
}

#define GRAPHICS_SHADER L"../data/fullQuad.fx" 
/**
 *	Load full screen quad for rendering both src and dest texture.
 */
bool DX11EffectViewer::InitGraphics(ID3D11Device* pd3dDevice)
{
	HRESULT hr;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined( DEBUG ) || defined( _DEBUG )
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
	
	ID3DBlob* pErrorBlob = NULL;
	ID3DBlob* pVSBlob = NULL;
	D3D11_COMPILE_CALL_CHECK(D3DCompileFromFile(GRAPHICS_SHADER, NULL, NULL, "VS", "vs_4_0", dwShaderFlags, 0, &pVSBlob, &pErrorBlob));
	D3D11_CALL_CHECK(pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVertexShader));
	if( pErrorBlob ) pErrorBlob->Release(),pErrorBlob = nullptr ; // is this check a must ?
	if (pVSBlob) pVSBlob->Release(), pVSBlob = nullptr;

	Info("- Create  vertex Shader OK.\n");

	// Compile pixel shader
	ID3DBlob* pPSBlob = nullptr;
	D3D11_COMPILE_CALL_CHECK(D3DCompileFromFile(GRAPHICS_SHADER, NULL, NULL, "psSampleSrcImage", "ps_4_0", dwShaderFlags, 0, &pPSBlob, &pErrorBlob));
	D3D11_CALL_CHECK(pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShaderSrcImage ));
	if (pPSBlob) pPSBlob->Release(), pPSBlob = nullptr;
	if (pErrorBlob) pErrorBlob->Release(), pErrorBlob = nullptr;

	// Compile pixel shader
	D3D11_COMPILE_CALL_CHECK ( D3DCompileFromFile(GRAPHICS_SHADER, NULL, NULL, "psSampleResultImage", "ps_4_0", dwShaderFlags, 0, &pPSBlob, &pErrorBlob)) ;
	D3D11_CALL_CHECK(pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShaderResultImage));
	if (pPSBlob) pPSBlob->Release(), pPSBlob = nullptr;
	if (pErrorBlob) pErrorBlob->Release(), pErrorBlob = nullptr;

	Info("- Create  pixel Shader OK.\n");

	// Create sampler state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory( &sampDesc, sizeof(sampDesc) );
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0; sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	D3D11_CALL_CHECK(hr = pd3dDevice->CreateSamplerState( &sampDesc, &m_pSamplerLinear ));
	
	Info("InitGraphics OK. @%s:%d\n", __FILE__, __LINE__);
}

void    DX11EffectViewer::UpdateCSConstBuffer()
{
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    m_pImmediateContext->Map(m_GPUConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    auto pData = reinterpret_cast<CB*>(MappedResource.pData);
    pData->iHeight = this->m_imageHeight;
    pData->iWidth  = this->m_imageWidth;
    m_pImmediateContext->Unmap(m_GPUConstBuffer, 0);
	Info("- Update Constant buffer.\n");
}

bool DX11EffectViewer::CreateCSConstBuffer(ID3D11Device* pd3dDevice)
{
	D3D11_BUFFER_DESC descConstBuffer;
	ZeroMemory(&descConstBuffer, sizeof(descConstBuffer));
	descConstBuffer.ByteWidth = ((sizeof(CB) + 15) / 16) * 16; // Caution! size needs to be multiple of 16.
	descConstBuffer.Usage = D3D11_USAGE_DYNAMIC;
	descConstBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	descConstBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	// Fill in the subresource data.
	CB cb = { m_imageWidth, m_imageHeight };
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &cb;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	D3D11_CALL_CHECK(pd3dDevice->CreateBuffer(&descConstBuffer, &InitData, &m_GPUConstBuffer));
	m_pImmediateContext->CSSetConstantBuffers(0, 1, &m_GPUConstBuffer);

	Info("- Create Constant buffer and Bind to CS OK.\n");
	return true;
}

void  DX11EffectViewer::SetupViewport(float topLeftX, float topLeftY, int width, int height)
{
	D3D11_VIEWPORT vp;
	vp.Width  = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.TopLeftX = topLeftX; vp.TopLeftY = topLeftY;
	vp.MinDepth = 0.0f; vp.MaxDepth = 1.0f;
	m_pImmediateContext->RSSetViewports( 1, &vp );
}

bool DX11EffectViewer::CreateResultImageTextureAndView(ID3D11Device* pd3dDevice)
{
    if (m_resultImageTexture)m_resultImageTexture->Release(), m_resultImageTexture = NULL;
    D3D11_TEXTURE2D_DESC desc;
    m_srcImageTexture->GetDesc(&desc);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.MipLevels = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	D3D11_CALL_CHECK(pd3dDevice->CreateTexture2D(&desc, NULL, &m_resultImageTexture));

    if (m_resultImageTextureView)m_resultImageTextureView->Release(), m_resultImageTextureView = NULL;
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
    ZeroMemory(&viewDesc, sizeof(viewDesc));
    viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
    viewDesc.Texture2D.MipLevels = 1;
    viewDesc.Texture2D.MostDetailedMip = 0;
	D3D11_CALL_CHECK(pd3dDevice->CreateShaderResourceView(m_resultImageTexture, &viewDesc, &m_resultImageTextureView));
	Info("- Create result image texture & view OK.\n");
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////
////// https://github.com/Microsoft/DirectXTK/wiki/WICTextureLoader
//////
//////////////////////////////////////////////////////////////////////////////////////////////////

bool DX11EffectViewer::LoadImageAsSrcTexture(ID3D11Device* pd3dDevice)
{
    if (m_srcImageTexture)      m_srcImageTexture->Release(), m_srcImageTexture = NULL;
    if (m_srcImageTextureView)  m_srcImageTextureView->Release(), m_srcImageTextureView = NULL;

	D3D11_CALL_CHECK(CreateWICTextureFromFile(pd3dDevice, m_imageFilename, (ID3D11Resource **)&m_srcImageTexture, &m_srcImageTextureView));
	{
		D3D11_TEXTURE2D_DESC desc;
		m_srcImageTexture->GetDesc(&desc);
		if (desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM)
		{
		    Info("Load [%s]: Texture format NOT qualified: DXGI_FORMAT_R8G8B8A8_UNORM @%s:%d\n", m_imageName.c_str(),  __FILE__, __LINE__);
			return false;
		}
		m_imageWidth  = desc.Width;
		m_imageHeight = desc.Height;
		m_Aspect = double(m_imageWidth) / double(m_imageHeight);
        m_textureSizeInBytes = m_imageWidth * m_imageHeight * 4;
		Info("Load [%s]:size (%d,%d) OK.\n", m_imageName.c_str(), m_imageWidth, m_imageHeight);
	}
	return true;
}

bool DX11EffectViewer::CreateCSInputTextureView(ID3D11Device* pd3dDevice)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
    ZeroMemory(&viewDesc, sizeof(viewDesc));
    viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
    viewDesc.Texture2D.MipLevels = 1;
    viewDesc.Texture2D.MostDetailedMip = 0;
    if (tempCSInputTextureView)tempCSInputTextureView->Release(), tempCSInputTextureView = NULL;
    D3D11_CALL_CHECK(pd3dDevice->CreateShaderResourceView(m_srcImageTexture, &viewDesc, &tempCSInputTextureView));
	return true;
}

 /*
  *	Create a GPU buffer to feed the compute shader.
  */
bool DX11EffectViewer::CreateCSInputTextureAndView(ID3D11Device* pd3dDevice)
{
	D3D11_TEXTURE2D_DESC desc;
	m_srcImageTexture->GetDesc(&desc);
	D3D11_CALL_CHECK(pd3dDevice->CreateTexture2D(&desc, NULL, &tempCSInputTexture));
	
    m_pImmediateContext->CopyResource(tempCSInputTexture, m_srcImageTexture); // copy resource by GPU.

    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
    ZeroMemory(&viewDesc, sizeof(viewDesc));
    viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
    viewDesc.Texture2D.MipLevels = 1;
    viewDesc.Texture2D.MostDetailedMip = 0;
    D3D11_CALL_CHECK(pd3dDevice->CreateShaderResourceView(m_srcImageTexture, &viewDesc, &tempCSInputTextureView));
    
	return true;
}

/**
 * compute shader output on a buffer with the same size with source. create a
 * GPU buffer and an unordered resource view.
*/
bool DX11EffectViewer::CreateCSOutputTextureAndView(ID3D11Device* pd3dDevice)
{
    if (tempCSOutputTexture) tempCSOutputTexture->Release(), tempCSOutputTexture = NULL;
    D3D11_TEXTURE2D_DESC desc;
    m_srcImageTexture->GetDesc(&desc);
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	D3D11_CALL_CHECK(pd3dDevice->CreateTexture2D(&desc, NULL, &tempCSOutputTexture));
	
    if (tempCSOutputTextureView)tempCSOutputTextureView->Release(), tempCSOutputTextureView = NULL;

	D3D11_UNORDERED_ACCESS_VIEW_DESC descView;
    ZeroMemory(&descView, sizeof(descView));
    descView.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    descView.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    descView.Texture2D = { 0 };
    D3D11_CALL_CHECK(pd3dDevice->CreateUnorderedAccessView(tempCSOutputTexture, &descView, &tempCSOutputTextureView));
	return true;
}

void DX11EffectViewer::SaveResult()
{
	stbi_write_png("output.png", m_imageWidth, m_imageHeight, 4, GetResultImage(), m_imageWidth * 4);
}

/**
 *	Get a copy of the GPU dest buffer.
 *	// Resources usage. https://msdn.microsoft.com/en-us/library/windows/desktop/ff476259(v=vs.85).aspx
 */
byte* DX11EffectViewer::GetResultImage()
{
#if 1 
	if (!m_resultGPUCopy)
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		m_resultImageTexture->GetDesc(&desc);
		desc.Usage = D3D11_USAGE_STAGING; // Support data copy from GPU to CPU.
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.BindFlags = 0;
		desc.MiscFlags = 0;
		D3D11_CALL_CHECK(m_pd3dDevice->CreateTexture2D(&desc, NULL, &m_resultGPUCopy));
		
	}
	if (!m_resultCPUCopy)
	{
		m_resultCPUCopy = new byte[m_textureSizeInBytes];
		if (!m_resultCPUCopy) Info("- New CPU buffer failed.\n");
	}

	m_pImmediateContext->CopyResource(m_resultGPUCopy, m_resultImageTexture); // Copy resource by GPU
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	D3D11_CALL_CHECK(m_pImmediateContext->Map(m_resultGPUCopy, 0, D3D11_MAP_READ, 0, &mappedResource));
	memcpy(m_resultCPUCopy, mappedResource.pData, m_textureSizeInBytes); // copy from GPU meory into CPU memory.
	m_pImmediateContext->Unmap(m_resultGPUCopy, 0);
	return m_resultCPUCopy; // return CPU copy of GPU resource.

#endif
    return 0;
}

void DX11EffectViewer::WindowMessageCallback(void* context, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DX11EffectViewer *app = (DX11EffectViewer*)context;
	switch (message)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;
		}
		case WM_KEYUP:
		{
			char key = tolower((int)wParam);
			if (wParam == VK_F1)
			{
				std::string name;
				app->NextEffect(name);
			}
			else if (wParam == VK_F2)
			{
				std::string name;
				app->PrevEffect(name);
			}
			else if (wParam == VK_F3)
			{
				std::string name;
				app->NextImage(name);
			}
			else if (wParam == VK_ESCAPE)
			{
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
			else if (key == 'u')
			{
				std::string name;
				app->UpdateEffects();
				app->NextEffect(name);
			}
			else if (key == 'd')
			{
				app->mDisplayMode = DisplayMode((1 + app->mDisplayMode) % DisplayMode::ALL_MODE);
			}
			else if (key == 'q')
			{
				PostQuitMessage(0);
			}
			break;
		}
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	DX11EffectViewer viewer;
	viewer.Run();
	return 0;
}