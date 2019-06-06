#include "DX11EffectViewer.h"
#include "WICTextureLoader.h"
#include "EffectManager.h"
#include "Logger.h"
#include "Utils.h"

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

int	DX11EffectViewer::initialize(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pImmediateContext)
{
    m_pd3dDevice = pd3dDevice;
    m_pImmediateContext = pImmediateContext;

    
	LoadImageAsSrcTexture(pd3dDevice); //< Load source image as texture and upate image size.
    CreateResultImageTextureAndView(pd3dDevice);
	InitGraphics(pd3dDevice);

	CreateCSConstBuffer(pd3dDevice);
	CreateCSInputTextureAndView(pd3dDevice);
	CreateCSOutputTextureAndView(pd3dDevice);

    BuildImageList(IMAGE_REPO);

    EffectManager::GetEffectManager(pd3dDevice)->CheckEffect();
	Error("[%s]: DX11EffectViewer Initialized OK @%s:%d\n", m_imageName.c_str(), __FILE__, __LINE__);
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


void DX11EffectViewer::Render(ID3D11DeviceContext* pImmediateContext ) 
{
    assert(pImmediateContext == m_pImmediateContext);

	UINT offset = 0, stride = sizeof( SimpleVertex );
	m_pImmediateContext->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &stride, &offset );
	m_pImmediateContext->IASetInputLayout( m_pVertexLayout );
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
        m_pImmediateContext->Draw( 4, 0 );// draw non-indexed non-instanced primitives.[vertex count, vertex offset in vertex buffer]

        m_pImmediateContext->PSSetShaderResources(1, 1, &m_resultImageTextureView );
        m_pImmediateContext->PSSetShader( m_pPixelShaderResultImage, NULL, 0 );
        SetupViewport(m_imageWidth + 1.f, 0.f, m_imageWidth, m_imageHeight);
        m_pImmediateContext->Draw( 4, 0 );// draw non-indexed non-instanced primitives.[vertex count, vertex offset in vertex buffer]
    }
}

void DX11EffectViewer::RenderMultiViewport()
{
	int width, height;
	width = float(widthSwapchain - 2) / 2.;
	height = 1. / m_Aspect * width;
	m_pImmediateContext->PSSetShaderResources( 0, 1, &m_srcImageTextureView );
	m_pImmediateContext->PSSetShader( m_pPixelShaderSrcImage, NULL, 0 );
	SetupViewport(0.f, 0.f, width, height);
	m_pImmediateContext->Draw( 4, 0 );

	m_pImmediateContext->PSSetShaderResources( 1, 1, &m_resultImageTextureView );
	m_pImmediateContext->PSSetShader( m_pPixelShaderResultImage, NULL, 0 );
	SetupViewport(width + 2.f, 0.f, width, height);
	m_pImmediateContext->Draw( 4, 0 );
}

void	DX11EffectViewer::RenderSourceImage()
{
	m_pImmediateContext->PSSetShaderResources( 0, 1, &m_srcImageTextureView );
	m_pImmediateContext->PSSetShader( m_pPixelShaderSrcImage, NULL, 0 );
	SetupViewport(0.f, 0.f, m_imageWidth, m_imageHeight);
	m_pImmediateContext->Draw( 4, 0 );
}

void	DX11EffectViewer::RenderResultImage()
{
	m_pImmediateContext->PSSetShaderResources( 1, 1, &m_resultImageTextureView );
	m_pImmediateContext->PSSetShader( m_pPixelShaderResultImage, NULL, 0 );
	SetupViewport(0.f, 0.f, m_imageWidth, m_imageHeight);
	m_pImmediateContext->Draw( 4, 0 );
}

void DX11EffectViewer::Destory() 
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
	SafeRelease(&m_pImmediateContext);
	SafeRelease(&m_pd3dDevice);
}

#define GRAPHICS_SHADER L"../data/fullQuad.fx" 
/**
 *	Load full screen quad for rendering both src and dest texture.
 */
void DX11EffectViewer::InitGraphics(ID3D11Device* pd3dDevice)
{
	HRESULT hr;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined( DEBUG ) || defined( _DEBUG )
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
	
	SimpleVertex vertices[] =
	{
		{  XMFLOAT3(-1.0f,-1.0f, 0.5f ), XMFLOAT2( 0.0f, 1.0f ) },
		{  XMFLOAT3(-1.0f, 1.0f, 0.5f ), XMFLOAT2( 0.0f, 0.0f ) },
		{  XMFLOAT3( 1.0f,-1.0f, 0.5f ), XMFLOAT2( 1.0f, 1.0f ) },
		{  XMFLOAT3( 1.0f, 1.0f, 0.5f ), XMFLOAT2( 1.0f, 0.0f ) }
	};

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = vertices;

	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.Usage     = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER; //bind the buffer to input-assembler stage.
	bd.ByteWidth = sizeof( vertices);
	hr = pd3dDevice->CreateBuffer( &bd, &InitData, &m_pVertexBuffer );
	if( FAILED( hr ) )
	{	
	    Error("[%s]: Failed to create vertex buffer @%s:%d\n", m_imageName.c_str(), __FILE__, __LINE__);
		exit(1);
	}

	ID3DBlob* pErrorBlob = NULL;
	ID3DBlob* pVSBlob = NULL;
	if( FAILED(D3DCompileFromFile(GRAPHICS_SHADER, NULL, NULL, "VS", "vs_4_0", dwShaderFlags, 0, &pVSBlob, &pErrorBlob) ) )
	{
		if( pErrorBlob )
		{
			OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
			pErrorBlob->Release();
		}
	    Error("[%s]: Failed to compile vertex shader: %s @%s:%d\n", m_imageName.c_str(),GRAPHICS_SHADER, __FILE__, __LINE__);
		exit(1);
	}
	if( pErrorBlob ) pErrorBlob->Release(); // is this check a must ?
	hr = pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVertexShader );
	if( FAILED(hr) )
	{	
	    Error("[%s]: Failed to create vertex shader object., %s @%s:%d\n", m_imageName.c_str(),GRAPHICS_SHADER, __FILE__, __LINE__);
		exit(1);
	}

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	hr = pd3dDevice->CreateInputLayout(layout, 2, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr))
	{
	    Error("[%s]: Failed to layout object, %s @%s:%d\n", m_imageName.c_str(),GRAPHICS_SHADER, __FILE__, __LINE__);
		exit(1);
	}

	// Compile pixel shader
	ID3DBlob* pPSBlob = NULL;
	if( FAILED( D3DCompileFromFile(GRAPHICS_SHADER, NULL, NULL, "PS", "ps_4_0", dwShaderFlags, 0, &pPSBlob, &pErrorBlob) ) )
	{
		if( pErrorBlob )
		{
			OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
			pErrorBlob->Release();
		}
	    Error("[%s]: Failed to compile src image pixel shader, %s @%s:%d\n", m_imageName.c_str(),GRAPHICS_SHADER, __FILE__, __LINE__);
		exit(1);
	}
	hr = pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShaderSrcImage );
	pPSBlob->Release();
	if( FAILED( hr ) )
	{	
	    Error("[%s]: Failed to create src image pixel shader, %s @%s:%d\n", m_imageName.c_str(),GRAPHICS_SHADER, __FILE__, __LINE__);
		exit(1);
	}

	// Compile pixel shader
	if( FAILED( D3DCompileFromFile(GRAPHICS_SHADER, NULL, NULL, "psSampleResultImage", "ps_4_0", dwShaderFlags, 0, &pPSBlob, &pErrorBlob) ) )
	{
		if( pErrorBlob )
		{
			OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
			pErrorBlob->Release();
		}
	    Error("[%s]: Failed to compile result image pixel shader, %s @%s:%d\n", m_imageName.c_str(),GRAPHICS_SHADER, __FILE__, __LINE__);
		exit(1);
	}
	hr = pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShaderResultImage );
	pPSBlob->Release();
	if( FAILED( hr ) )
	{	
	    Error("[%s]: Failed to create pixel shader object @%s:%d\n", m_imageName.c_str(),  __FILE__, __LINE__);
		exit(1);
	}

	// Create sampler state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory( &sampDesc, sizeof(sampDesc) );
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0; sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = pd3dDevice->CreateSamplerState( &sampDesc, &m_pSamplerLinear );
	if (FAILED(hr))
	{
	    Info("[%s]: Failed to Create Texture Sampler Object. @%s:%d\n", m_imageName.c_str(),  __FILE__, __LINE__);
		exit(1);
	}
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

}

void    DX11EffectViewer::CreateCSConstBuffer(ID3D11Device* pd3dDevice)
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
	HRESULT hr = pd3dDevice->CreateBuffer(&descConstBuffer, &InitData, &m_GPUConstBuffer); // create const buffer.
	if (FAILED(hr))
	{
		Info("[%s]: Create Parameter Constant Buffer Failed @%s:%d\n", m_imageName.c_str(),  __FILE__, __LINE__);
		exit(1);
	}
	m_pImmediateContext->CSSetConstantBuffers(0, 1, &m_GPUConstBuffer);
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

void   DX11EffectViewer::CreateResultImageTextureAndView(ID3D11Device* pd3dDevice)
{
    if (m_resultImageTexture)m_resultImageTexture->Release(), m_resultImageTexture = NULL;
    D3D11_TEXTURE2D_DESC desc;
    m_srcImageTexture->GetDesc(&desc);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.MipLevels = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    if ( FAILED( pd3dDevice->CreateTexture2D(&desc, NULL, &m_resultImageTexture)))
    {
		Info("[%s]: Failed to create result image texture @%s:%d\n", m_imageName.c_str(),  __FILE__, __LINE__);
		exit(1);
    }

    if (m_resultImageTextureView)m_resultImageTextureView->Release(), m_resultImageTextureView = NULL;
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
    ZeroMemory(&viewDesc, sizeof(viewDesc));
    viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
    viewDesc.Texture2D.MipLevels = 1;
    viewDesc.Texture2D.MostDetailedMip = 0;
    if (FAILED(pd3dDevice->CreateShaderResourceView(m_resultImageTexture, &viewDesc, &m_resultImageTextureView)))
    {
		Info("[%s]: Failed to create result image texture resource view @%s:%d\n", m_imageName.c_str(),  __FILE__, __LINE__);
		exit(1);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////
////// https://github.com/Microsoft/DirectXTK/wiki/WICTextureLoader
//////
//////////////////////////////////////////////////////////////////////////////////////////////////

void DX11EffectViewer::LoadImageAsSrcTexture(ID3D11Device* pd3dDevice)
{
    if (m_srcImageTexture)      m_srcImageTexture->Release(), m_srcImageTexture = NULL;
    if (m_srcImageTextureView)  m_srcImageTextureView->Release(), m_srcImageTextureView = NULL;

    if (SUCCEEDED(CreateWICTextureFromFile(pd3dDevice, m_imageFilename, (ID3D11Resource **)&m_srcImageTexture, &m_srcImageTextureView)))
	{
		D3D11_TEXTURE2D_DESC desc;
		m_srcImageTexture->GetDesc(&desc);
		if (desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM)
		{
		    Info("[%s]: Texture format is not qualified: DXGI_FORMAT_R8G8B8A8_UNORM @%s:%d\n", m_imageName.c_str(),  __FILE__, __LINE__);
			exit(1);
		}
		m_imageWidth  = desc.Width;
		m_imageHeight = desc.Height;
		m_Aspect = double(m_imageWidth) / double(m_imageHeight);
        m_textureDataSize = m_imageWidth * m_imageHeight * 4;
		Info("size [%s]:(%d,%d) @%s:%d\n", m_imageName.c_str(), m_imageWidth, m_imageHeight, __FILE__, __LINE__);
	}
	else
	{
		Info("size [%s]: load failed @%s:%d\n", m_imageName.c_str(),  __FILE__, __LINE__);
		exit(1);
	}
}

void DX11EffectViewer::CreateCSInputTextureView(ID3D11Device* pd3dDevice)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
    ZeroMemory(&viewDesc, sizeof(viewDesc));
    viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
    viewDesc.Texture2D.MipLevels = 1;
    viewDesc.Texture2D.MostDetailedMip = 0;
    if (tempCSInputTextureView)tempCSInputTextureView->Release(), tempCSInputTextureView = NULL;
    if (FAILED(pd3dDevice->CreateShaderResourceView(m_srcImageTexture, &viewDesc, &tempCSInputTextureView)))
    {
		Info("[%s]: Failed to create compute shader SRV. @%s:%d\n", m_imageName.c_str(),  __FILE__, __LINE__);
		exit(1);
    }
}

 /*
  *	Once we have the texture data in RAM we create a GPU buffer to feed the compute shader.
  */
void DX11EffectViewer::CreateCSInputTextureAndView(ID3D11Device* pd3dDevice)
{
	D3D11_TEXTURE2D_DESC desc;
	m_srcImageTexture->GetDesc(&desc);
	if (pd3dDevice->CreateTexture2D(&desc, NULL, &tempCSInputTexture) != S_OK)
	{
		Info("[%s]: Failed to create compute shader input texture @%s:%d\n", m_imageName.c_str(),  __FILE__, __LINE__);
		exit(1);
	}
    m_pImmediateContext->CopyResource(tempCSInputTexture, m_srcImageTexture); // copy resource by GPU.

    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
    ZeroMemory(&viewDesc, sizeof(viewDesc));
    viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
    viewDesc.Texture2D.MipLevels = 1;
    viewDesc.Texture2D.MostDetailedMip = 0;
    //if (FAILED(m_pd3dDevice->CreateShaderResourceView(tempCSInputTexture, &viewDesc, &tempCSInputTextureView)))
    if (FAILED(pd3dDevice->CreateShaderResourceView(m_srcImageTexture, &viewDesc, &tempCSInputTextureView)))
    {
		Info("[%s]: Failed to create compute shader input SRV @%s:%d\n", m_imageName.c_str(),  __FILE__, __LINE__);
		exit(1);
    }
}

/**
*	We know the compute shader will output on a buffer which is as big as the texture. Therefore we need to create a
*	GPU buffer and an unordered resource view.
*/
void DX11EffectViewer::CreateCSOutputTextureAndView(ID3D11Device* pd3dDevice)
{
    if (tempCSOutputTexture) tempCSOutputTexture->Release(), tempCSOutputTexture = NULL;
    D3D11_TEXTURE2D_DESC desc;
    m_srcImageTexture->GetDesc(&desc);
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	if (pd3dDevice->CreateTexture2D(&desc, NULL, &tempCSOutputTexture) != S_OK)
	{
		Info("[%s]: Failed to create compute shader input texture @%s:%d\n", m_imageName.c_str(),  __FILE__, __LINE__);
		exit(1);
	}

    if (tempCSOutputTextureView)tempCSOutputTextureView->Release(), tempCSOutputTextureView = NULL;
	D3D11_UNORDERED_ACCESS_VIEW_DESC descView;
    ZeroMemory(&descView, sizeof(descView));
    descView.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    descView.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    descView.Texture2D = { 0 };
    if (FAILED(pd3dDevice->CreateUnorderedAccessView(tempCSOutputTexture, &descView, &tempCSOutputTextureView)))
    {
		Info("[%s]: Failed to create compute shader output SRV @%s:%d\n", m_imageName.c_str(),  __FILE__, __LINE__);
		exit(1);
    }
}

/*
 *	Load a compute shader from  file and use CSMain as entry point
 */
void DX11EffectViewer::LoadComputeShader(LPCWSTR filename, LPCSTR entrypoint, ID3D11ComputeShader** computeShader)
{
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined( _DEBUG )
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob = NULL;
	ID3DBlob* pBlob = NULL;
    LPCSTR pTarget = ( m_pd3dDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0 ) ? "cs_5_0" : "cs_4_0";
	HRESULT hr = D3DCompileFromFile( filename, NULL, NULL, entrypoint, pTarget, dwShaderFlags, NULL, &pBlob, &pErrorBlob);
	if ( FAILED(hr) )
	{
		if ( pErrorBlob ) OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
		if ( pErrorBlob ) pErrorBlob->Release();
		if(pBlob) pBlob->Release();
		Info("[%s]: Compile Compute Shader Failed @%s:%d\n", m_imageName.c_str(),  __FILE__, __LINE__);
		exit(2);
	}
	else
	{
		hr = m_pd3dDevice->CreateComputeShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, computeShader );
		if (pErrorBlob) pErrorBlob->Release();
		if (pBlob) pBlob->Release();
	}
}

/**
 *	Get a copy of the GPU dest buffer.
 *	// Resources usage. https://msdn.microsoft.com/en-us/library/windows/desktop/ff476259(v=vs.85).aspx
 */
byte* DX11EffectViewer::getCPUCopyOfGPUDestBuffer()
{
#if 0 
	if (!m_dstDataBufferGPUCopy)
	{
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		m_destDataGPUBuffer->GetDesc(&desc);
		desc.Usage = D3D11_USAGE_STAGING; // Support data copy from GPU to CPU.
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.BindFlags = 0;
		desc.MiscFlags = 0;
		if ( FAILED(m_pd3dDevice->CreateBuffer(&desc, NULL, &m_dstDataBufferGPUCopy)))
		{
			printf("- Create copy of Compute output buffer failed.\n");
			return NULL;
		}
	}
	if (!m_dstDataBufferCPUCopy)
	{
		m_dstDataBufferCPUCopy = new byte[m_textureDataSize];
		if (!m_dstDataBufferCPUCopy) printf("- Allocaate dst data buffer cpu copy failed.\n");
	}

	m_pImmediateContext->CopyResource(m_dstDataBufferGPUCopy, m_destDataGPUBuffer ); // Copy resource by GPU
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if(m_pImmediateContext->Map(m_dstDataBufferGPUCopy, 0, D3D11_MAP_READ, 0, &mappedResource) != S_OK) return false;
	memcpy(m_dstDataBufferCPUCopy, mappedResource.pData, m_textureDataSize); // copy from GPU meory into CPU memory.
	m_pImmediateContext->Unmap(m_dstDataBufferGPUCopy, 0);
	return m_dstDataBufferCPUCopy; // return CPU copy of GPU resource.
#endif
    return 0;
}