#include "DXApplication.h"
#include "WICTextureLoader.h"


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

/**
 *	Initialize our DX application with windows size.
 */
bool DXApplication::initialize(HWND hWnd ) 
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
		printf("- Create D3D Device and Swap Chain Failed.\n");
		return false;
	}

	// Create Render Target View Object from SwapChain's Back Buffer.
	// access one of swap chain's back buffer.[0-based buffer index, interface type which manipulates buffer, output param]
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
	if (FAILED(hr))
	{
		printf("- Cet Back Buffer from SwapChain Failed.\n");
		return false;
	}
	hr = m_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &m_pRenderTargetView );
	pBackBuffer->Release();
	if (FAILED(hr))
	{
		printf("- Create render target from Back buffer failed.\n");
		return false;
	}

	LoadImageAsTexture();// Load texture and upate image size.
    CreateResultImageTextureAndView();
	InitGraphics();

	CreateCSConstBuffer();
	CreateCSInputTextureAndView();
	CreateCSOutputTextureAndView();
	return true;
}

void DXApplication::RunComputeShader( ) 
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
	
	m_pImmediateContext->Dispatch( (m_imageWidth + 31) / 32, (m_imageHeight + 31) / 32, 1 );// So Dispatch returns immediately?

	m_pImmediateContext->CSSetShader( NULL, NULL, 0 );
	m_pImmediateContext->CSSetUnorderedAccessViews( 0, 1, ppUAViewNULL, NULL );
	m_pImmediateContext->CSSetShaderResources( 0, 1, ppSRVNULL );

    m_pImmediateContext->CopyResource(m_resultImageTexture, tempCSOutputTexture);
}

void DXApplication::Render() 
{
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
        float ClearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
        m_pImmediateContext->ClearRenderTargetView( m_pRenderTargetView, ClearColor );
        m_pImmediateContext->PSSetShaderResources( 0, 1, &m_srcImageTextureView );
        m_pImmediateContext->PSSetShader( m_pPixelShader, NULL, 0 );
        SetupViewport(0.f, 0.f, m_imageWidth, m_imageHeight);
        m_pImmediateContext->Draw( 4, 0 );// draw non-indexed non-instanced primitives.[vertex count, vertex offset in vertex buffer]

        m_pImmediateContext->PSSetShaderResources( 1, 1, &m_resultImageTextureView );
        SetupViewport(m_imageWidth, m_imageHeight, m_imageWidth, m_imageHeight);
        m_pImmediateContext->PSSetShader( m_pPixelShaderResultImage, NULL, 0 );
        m_pImmediateContext->Draw( 4, 0 );// draw non-indexed non-instanced primitives.[vertex count, vertex offset in vertex buffer]

        m_pSwapChain->Present( 0, 0 );
    }
}

void DXApplication::RenderMultiViewport()
{
	float ClearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	m_pImmediateContext->ClearRenderTargetView( m_pRenderTargetView, ClearColor );
	m_pImmediateContext->PSSetShaderResources( 0, 1, &m_srcImageTextureView );
	m_pImmediateContext->PSSetShader( m_pPixelShader, NULL, 0 );
	SetupViewport(0.f, 0.f, m_imageWidth, m_imageHeight);
	m_pImmediateContext->Draw( 4, 0 );

	m_pImmediateContext->PSSetShaderResources( 1, 1, &m_resultImageTextureView );
	SetupViewport(m_imageWidth, m_imageHeight, m_imageWidth, m_imageHeight);
	m_pImmediateContext->PSSetShader( m_pPixelShaderResultImage, NULL, 0 );
	m_pImmediateContext->Draw( 4, 0 );

	m_pSwapChain->Present( 0, 0 );
}

void	DXApplication::RenderSourceImage()
{
	float ClearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	m_pImmediateContext->ClearRenderTargetView( m_pRenderTargetView, ClearColor );
	m_pImmediateContext->PSSetShaderResources( 0, 1, &m_srcImageTextureView );
	m_pImmediateContext->PSSetShader( m_pPixelShader, NULL, 0 );
	SetupViewport(0.f, 0.f, m_imageWidth, m_imageHeight);
	m_pImmediateContext->Draw( 4, 0 );
	m_pSwapChain->Present( 0, 0 );
}

void	DXApplication::RenderResultImage()
{
	float ClearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	m_pImmediateContext->ClearRenderTargetView( m_pRenderTargetView, ClearColor );
	m_pImmediateContext->PSSetShaderResources( 1, 1, &m_resultImageTextureView );
	SetupViewport(0.f, 0.f, m_imageWidth, m_imageHeight);
	m_pImmediateContext->PSSetShader( m_pPixelShaderResultImage, NULL, 0 );
	m_pImmediateContext->Draw( 4, 0 );

	m_pSwapChain->Present( 0, 0 );
}

void DXApplication::release() 
{
	if(m_srcTextureData) delete [] m_srcTextureData;
	m_srcTextureData = NULL;

	SafeRelease(&m_computeShader);

	SafeRelease(&m_resultImageTexture);
	SafeRelease(&m_resultImageTextureView);

	SafeRelease(&m_srcImageTexture);
	SafeRelease(&m_srcImageTextureView);

	SafeRelease(&m_pVertexShader);
	SafeRelease(&m_pPixelShader);
	SafeRelease(&m_pVertexLayout);
	SafeRelease(&m_pVertexBuffer);
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
void DXApplication::InitGraphics()
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
	hr = m_pd3dDevice->CreateBuffer( &bd, &InitData, &m_pVertexBuffer );
	if( FAILED( hr ) )
	{	
		printf( "- Failed to create vertex buffer.\n" );
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
		exit(1);
	}
	if( pErrorBlob ) pErrorBlob->Release(); // is this check a must ?
	hr = m_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVertexShader );
	if( FAILED(hr) )
	{	
		printf( "- Failed to create vertex shader.\n" );
		exit(1);
	}

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	hr = m_pd3dDevice->CreateInputLayout(layout, 2, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr))
	{
		printf("- Failed to create input layout object.\n");
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
		exit(1);
	}
	hr = m_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShader );
	pPSBlob->Release();
	if( FAILED( hr ) )
	{	
		printf( "- Failed to create pixel shader. \n" );
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
		exit(1);
	}
	hr = m_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShaderResultImage );
	pPSBlob->Release();
	if( FAILED( hr ) )
	{	
		printf( "- Failed to create pixel shader. \n" );
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
	hr = m_pd3dDevice->CreateSamplerState( &sampDesc, &m_pSamplerLinear );
	if (FAILED(hr))
	{
		printf("- Failed to Create Texture Sampler Object.\n");
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
}


void    DXApplication::CreateCSConstBuffer()
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
	HRESULT hr = m_pd3dDevice->CreateBuffer(&descConstBuffer, &InitData, &m_GPUConstBuffer); // create const buffer.
	if (FAILED(hr))
	{
		printf("-  Create Constant Buffer Failed. \n");
		exit(1);
	}
	m_pImmediateContext->CSSetConstantBuffers(0, 1, &m_GPUConstBuffer);
}

void  DXApplication::SetupViewport(float topLeftX, float topLeftY, int width, int height)
{
	D3D11_VIEWPORT vp;
	vp.Width  = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.TopLeftX = topLeftX; vp.TopLeftY = topLeftY;
	vp.MinDepth = 0.0f; vp.MaxDepth = 1.0f;
	m_pImmediateContext->RSSetViewports( 1, &vp );
}

void   DXApplication::CreateResultImageTextureAndView()
{
    D3D11_TEXTURE2D_DESC desc;
    m_srcImageTexture->GetDesc(&desc);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.MipLevels = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    if ( FAILED( m_pd3dDevice->CreateTexture2D(&desc, NULL, &m_resultImageTexture)))
    {
        printf("- Failed to create result image texture and resource view.\n");
		exit(1);
    }

    // Create a view output texture
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
    ZeroMemory(&viewDesc, sizeof(viewDesc));
    viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
    viewDesc.Texture2D.MipLevels = 1;
    viewDesc.Texture2D.MostDetailedMip = 0;
    if (FAILED(m_pd3dDevice->CreateShaderResourceView(m_resultImageTexture, &viewDesc, &m_resultImageTextureView)))
    {
        printf("- Failed to create result image texture and resource view.\n");
		exit(1);
    }
}

/*
 * https://github.com/Microsoft/DirectXTK/wiki/WICTextureLoader
 */
void DXApplication::LoadImageAsTexture()
{
	if (SUCCEEDED(CreateWICTextureFromFile(m_pd3dDevice, m_imageFilename, (ID3D11Resource **)&m_srcImageTexture, &m_srcImageTextureView)))
	{
		D3D11_TEXTURE2D_DESC desc;
		m_srcImageTexture->GetDesc(&desc);
		if (desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM)
		{
			printf("- Texture format is not qualified.\n");
			exit(1);
		}
		// update image size. 
		m_imageWidth  = desc.Width;
		m_imageHeight = desc.Height;
        m_textureDataSize = m_imageWidth * m_imageHeight * 4;
		printf("- Texture loaded, size=%dx%d.\n", m_imageWidth, m_imageHeight);
	}
	else
	{
		printf("- Texture Load failed.\n");
		exit(1);
	}
}

/**
  *	Once we have the texture data in RAM we create a GPU buffer to feed the compute shader.
  */
void DXApplication::CreateCSInputTextureAndView()
{
	D3D11_TEXTURE2D_DESC desc;
	m_srcImageTexture->GetDesc(&desc);
	if (m_pd3dDevice->CreateTexture2D(&desc, NULL, &tempCSInputTexture) != S_OK)
	{
        printf("- Failed to create compute shader texture resource view.\n");
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
    if (FAILED(m_pd3dDevice->CreateShaderResourceView(m_srcImageTexture, &viewDesc, &tempCSInputTextureView)))
    {
        printf("- Failed to create compute shader texture resource view.\n");
		exit(1);
    }
}

/**
*	We know the compute shader will output on a buffer which is as big as the texture. Therefore we need to create a
*	GPU buffer and an unordered resource view.
*/
void DXApplication::CreateCSOutputTextureAndView()
{
    D3D11_TEXTURE2D_DESC desc;
    m_srcImageTexture->GetDesc(&desc);
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	if (m_pd3dDevice->CreateTexture2D(&desc, NULL, &tempCSOutputTexture) != S_OK)
	{
        printf("- Create Compute Shader output buffer view failed.");
		exit(1);
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC descView;
    ZeroMemory(&descView, sizeof(descView));
    descView.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    descView.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    descView.Texture2D = { 0 };
    if (FAILED(m_pd3dDevice->CreateUnorderedAccessView(tempCSOutputTexture, &descView, &tempCSOutputTextureView)))
    {
        printf("- Create Compute Shader output buffer view failed.");
		exit(1);
    }
}

/*
 *	Load a compute shader from  file and use CSMain as entry point
 */
void DXApplication::LoadComputeShader(LPCWSTR filename, LPCSTR entrypoint, ID3D11ComputeShader** computeShader)
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
byte* DXApplication::getCPUCopyOfGPUDestBuffer()
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