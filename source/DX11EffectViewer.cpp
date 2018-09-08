#include "DX11EffectViewer.h"
#include "WICTextureLoader.h"

#include "EffectManager.h"
#include "Logger.h"
#include <io.h>

// Safe Release Function
template <class T>
void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

static void getFiles(std::string path, std::vector<std::string>& files)
{
    long   hFile = 0;
    struct _finddata_t fileinfo;
    std::string p;
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
    {
        do
        {
            if ((fileinfo.attrib &  _A_SUBDIR)) // subdir
            {
            }
            else
            {
                files.push_back(p.assign(path).append("\\").append(fileinfo.name));
            }
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
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

	LoadImageAsTexture(pd3dDevice);// Load texture and upate image size.
    CreateResultImageTextureAndView(pd3dDevice);
	InitGraphics(pd3dDevice);

	CreateCSConstBuffer(pd3dDevice);
	CreateCSInputTextureAndView(pd3dDevice);
	CreateCSOutputTextureAndView(pd3dDevice);

    BuildImageList("C:\\Users\\ding\\Documents\\GitHub\\DX11ComputeShaderForImageFilters\\image");

    EffectManager::GetEffectManager(pd3dDevice)->CheckEffect();
    Logger::getLogger() << "- DX11EffectViewer Initialized OK. \n" << std::endl;
	return 0;
}

bool DX11EffectViewer::Initialize(HWND hWnd ) 
{
	HRESULT hr = S_OK;
    RECT rc;
    GetClientRect(hWnd, &rc);
    unsigned int width = rc.right - rc.left;
    unsigned int height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
	#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE( driverTypes );

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

	D3D_DRIVER_TYPE         driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL       featureLevel = D3D_FEATURE_LEVEL_11_0;

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof(sd) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width  = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	// Create Device, DeviceContext, SwapChain, FeatureLevel
	for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
	{
		driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain( NULL, driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, &featureLevel, &m_pImmediateContext );
		if( SUCCEEDED(hr) ) break;
	}
	if ( FAILED(hr) )
	{
        Logger::getLogger() << "- Create D3D Device and Swap Chain Failed." << "\n" << std::endl;
		return false;
	}

	// Create Render Target View Object from SwapChain's Back Buffer.
	// access one of swap chain's back buffer.[0-based buffer index, interface type which manipulates buffer, output param]
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
	if (FAILED(hr))
	{
        Logger::getLogger() << "- Get Back Buffer from SwapChain Failed." << "\n";
		return false;
	}
	hr = m_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &m_pRenderTargetView );
	pBackBuffer->Release();
	if (FAILED(hr))
	{
        Logger::getLogger() << "- Create render target from Back buffer failed.\n" << "\n";
		return false;
	}

	LoadImageAsTexture(m_pd3dDevice);// Load texture and upate image size.
    CreateResultImageTextureAndView(m_pd3dDevice);
	InitGraphics(m_pd3dDevice);

	CreateCSConstBuffer(m_pd3dDevice);
	CreateCSInputTextureAndView(m_pd3dDevice);
	CreateCSOutputTextureAndView(m_pd3dDevice);

    BuildImageList("C:\\Users\\ding\\Documents\\GitHub\\DX11ComputeShaderForImageFilters\\image");

    EffectManager::GetEffectManager(m_pd3dDevice)->CheckEffect();;
    Logger::getLogger() << "- DX11EffectViewer Initialized OK. \n" << std::endl;
	return true;
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
	
	m_pImmediateContext->Dispatch( (m_imageWidth + 31) / 32, (m_imageHeight + 31) / 32, 1 );// So Dispatch returns immediately?

	m_pImmediateContext->CSSetShader( NULL, NULL, 0 );
	m_pImmediateContext->CSSetUnorderedAccessViews( 0, 1, ppUAViewNULL, NULL );
	m_pImmediateContext->CSSetShaderResources( 0, 1, ppSRVNULL );

    m_pImmediateContext->CopyResource(m_resultImageTexture, tempCSOutputTexture);
}

void DX11EffectViewer::RunComputeShader( ) 
{
	ID3D11UnorderedAccessView *ppUAViewNULL[2] = { NULL, NULL };
	ID3D11ShaderResourceView  *ppSRVNULL[2]    = { NULL, NULL };

	LoadComputeShader(m_csShaderFilename, "CSMain", &m_computeShader);
    if (m_computeShader == NULL)
    {
        return;
    }
	m_pImmediateContext->CSSetShader( m_computeShader, NULL, 0 );
    m_pImmediateContext->CSSetShaderResources(0, 1, &tempCSInputTextureView);
    m_pImmediateContext->CSSetUnorderedAccessViews(0, 1, &tempCSOutputTextureView, NULL);
	m_pImmediateContext->CSSetConstantBuffers(0, 1, &m_GPUConstBuffer);
	
	m_pImmediateContext->Dispatch( (m_imageWidth + 31) / 32, (m_imageHeight + 31) / 32, 1 );// So Dispatch returns immediately?

	m_pImmediateContext->CSSetShader( NULL, NULL, 0 );
	m_pImmediateContext->CSSetUnorderedAccessViews( 0, 1, ppUAViewNULL, NULL );
	m_pImmediateContext->CSSetShaderResources( 0, 1, ppSRVNULL );

    m_pImmediateContext->CopyResource(m_resultImageTexture, tempCSOutputTexture);
}

void DX11EffectViewer::Render(ID3D11DeviceContext* pImmediateContext ) 
{
    assert(pImmediateContext == m_pImmediateContext);
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
        m_pImmediateContext->PSSetShader( m_pPixelShader, NULL, 0 );
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
	m_pImmediateContext->PSSetShaderResources( 0, 1, &m_srcImageTextureView );
	m_pImmediateContext->PSSetShader( m_pPixelShader, NULL, 0 );
	SetupViewport(0.f, 0.f, m_imageWidth, m_imageHeight);
	m_pImmediateContext->Draw( 4, 0 );

	m_pImmediateContext->PSSetShaderResources( 1, 1, &m_resultImageTextureView );
	m_pImmediateContext->PSSetShader( m_pPixelShaderResultImage, NULL, 0 );
	SetupViewport(m_imageWidth + 1.f, 0.f, m_imageWidth, m_imageHeight);
	m_pImmediateContext->Draw( 4, 0 );
}

void	DX11EffectViewer::RenderSourceImage()
{
	m_pImmediateContext->PSSetShaderResources( 0, 1, &m_srcImageTextureView );
	m_pImmediateContext->PSSetShader( m_pPixelShader, NULL, 0 );
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
	if(m_srcTextureData) delete [] m_srcTextureData;
	m_srcTextureData = NULL;

	SafeRelease(&m_computeShader);

	SafeRelease(&m_resultImageTexture);
	SafeRelease(&m_resultImageTextureView);

	SafeRelease(&m_srcImageTexture);
	SafeRelease(&m_srcImageTextureView);

	SafeRelease(&m_pVertexBuffer);
	SafeRelease(&m_pVertexLayout);
	SafeRelease(&m_pVertexShader);
	SafeRelease(&m_pPixelShader);

	SafeRelease(&m_pSamplerLinear);

	if( m_pImmediateContext ) m_pImmediateContext->ClearState();
	SafeRelease(&m_pRenderTargetView);
	SafeRelease(&m_pSwapChain);
	SafeRelease(&m_pImmediateContext);
	SafeRelease(&m_pd3dDevice);
}

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
	
	struct SimpleVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Tex;
	};
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
        Logger::getLogger() << "- Failed to create vertex buffer.\n" << "\n";
		exit(1);
	}

	ID3DBlob* pErrorBlob;
	ID3DBlob* pVSBlob = NULL;
	if( FAILED(D3DCompileFromFile(L"./data/fullQuad.fx", NULL, NULL, "VS", "vs_4_0", dwShaderFlags, 0, &pVSBlob, &pErrorBlob) ) )
	{
		if( pErrorBlob )
		{
			OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
			pErrorBlob->Release();
		}
        Logger::getLogger() << "- Failed to compile vertex shader: /data/fullQuad.fx\n" << "\n";
		exit(1);
	}
	if( pErrorBlob ) pErrorBlob->Release(); // is this check a must ?
	hr = pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVertexShader );
	if( FAILED(hr) )
	{	
        Logger::getLogger() << "- Failed to create vertex shader object." << "\n";
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
        Logger::getLogger() << "- Failed to layout object\n" << "\n";
		exit(1);
	}

	// Compile pixel shader
	ID3DBlob* pPSBlob = NULL;
	if( FAILED( D3DCompileFromFile(L"./data/fullQuad.fx", NULL, NULL, "PS", "ps_4_0", dwShaderFlags, 0, &pPSBlob, &pErrorBlob) ) )
	{
		if( pErrorBlob )
		{
			OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
			pErrorBlob->Release();
		}
        Logger::getLogger() << "- Failed to compile pixel shader: /data/fullQuad.fx\n" << "\n";
		exit(1);
	}
	hr = pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShader );
	pPSBlob->Release();
	if( FAILED( hr ) )
	{	
        Logger::getLogger() << "- Failed to create pixel shader object." << "\n";
		exit(1);
	}

	// Compile pixel shader
	if( FAILED( D3DCompileFromFile(L"./data/fullQuad.fx", NULL, NULL, "psSampleResultImage", "ps_4_0", dwShaderFlags, 0, &pPSBlob, &pErrorBlob) ) )
	{
		if( pErrorBlob )
		{
			OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
			pErrorBlob->Release();
		}
        Logger::getLogger() << "- Failed to compile pixel shader: /data/fullQuad.fx\n" << "\n";
		exit(1);
	}
	hr = pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShaderResultImage );
	pPSBlob->Release();
	if( FAILED( hr ) )
	{	
        Logger::getLogger() << "- Failed to create pixel shader object." << "\n";
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
        Logger::getLogger() << "- Failed to Create Texture Sampler Object.\n" << "\n";
		exit(1);
	}

	UINT offset = 0, stride = sizeof( SimpleVertex );
	m_pImmediateContext->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &stride, &offset );
	m_pImmediateContext->IASetInputLayout( m_pVertexLayout );
	m_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
	m_pImmediateContext->VSSetShader( m_pVertexShader, NULL, 0 );
	m_pImmediateContext->PSSetShader( m_pPixelShader, NULL, 0 );
	m_pImmediateContext->PSSetSamplers( 0, 1, &m_pSamplerLinear );
    m_pImmediateContext->CSSetSamplers( 0, 1, &m_pSamplerLinear );
	m_pImmediateContext->OMSetRenderTargets( 1, &m_pRenderTargetView, NULL );

    Logger::getLogger() << "- InitGraphics OK.\n" << "\n";
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
        Logger::getLogger() << "-  Create Parameter Constant Buffer Failed. \n" << "\n";
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

void   DX11EffectViewer::CreateCSOutputImageTextureView()
{
    if (m_resultImageTextureView)m_resultImageTextureView->Release(), m_resultImageTextureView = NULL;
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
    ZeroMemory(&viewDesc, sizeof(viewDesc));
    viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
    viewDesc.Texture2D.MipLevels = 1;
    viewDesc.Texture2D.MostDetailedMip = 0;
    if (FAILED(m_pd3dDevice->CreateShaderResourceView(tempCSOutputTexture, &viewDesc, &m_resultImageTextureView)))
    {
        Logger::getLogger() << "-  Failed to create cs output result image texture resource view.\n" << "\n";
		exit(1);
    }
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
        Logger::getLogger() << "-  Failed to create result image texture.\n" << "\n";
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
        Logger::getLogger() << "-  Failed to create result image texture resource view.\n" << "\n";
		exit(1);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////
////// https://github.com/Microsoft/DirectXTK/wiki/WICTextureLoader
//////
//////////////////////////////////////////////////////////////////////////////////////////////////

void DX11EffectViewer::LoadImageAsTexture(ID3D11Device* pd3dDevice)
{
    if (m_srcImageTexture)      m_srcImageTexture->Release(), m_srcImageTexture = NULL;
    if (m_srcImageTextureView)  m_srcImageTextureView->Release(), m_srcImageTextureView = NULL;

    if (SUCCEEDED(CreateWICTextureFromFile(pd3dDevice, m_imageFilename, (ID3D11Resource **)&m_srcImageTexture, &m_srcImageTextureView)))
	{
		D3D11_TEXTURE2D_DESC desc;
		m_srcImageTexture->GetDesc(&desc);
		if (desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM)
		{
            Logger::getLogger() << "-  Texture format of input image is not qualified: DXGI_FORMAT_R8G8B8A8_UNORM\n" << std::endl;
			exit(1);
		}
		m_imageWidth  = desc.Width;
		m_imageHeight = desc.Height;
        m_textureDataSize = m_imageWidth * m_imageHeight * 4;
        Logger::getLogger() << "-  Size of input image: " << m_imageWidth << " x " << m_imageHeight << "\n" << std::endl;
	}
	else
	{
        Logger::getLogger() << "-  Texture Load failed! \n" << std::endl;
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
        Logger::getLogger() << "- Failed to create compute shader input texture resource view." << std::endl;
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
        Logger::getLogger() << "- Failed to create compute shader input texture.\n" << std::endl;
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
        Logger::getLogger() << "- Failed to create compute shader input texture resource view.\n" << std::endl;
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
        Logger::getLogger() << "- Failed to create compute shader input texture.\n" << std::endl;
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
        Logger::getLogger() << "- Failed to create compute shader output texture resource view.\n" << std::endl;
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
		printf("- Compile Compute Shader Failed.\n");
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