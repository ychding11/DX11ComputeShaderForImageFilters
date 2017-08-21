#include "DXApplication.h"
#include "WICTextureLoader.h"


// Safe Release Function
template <class T> void SafeRelease(T **ppT)
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
bool DXApplication::initialize(HWND hWnd, int width, int height) 
{
	HRESULT hr = S_OK;
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
	ZeroMemory( &sd, sizeof( sd ) );
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

	for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
	{
		driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain( NULL, driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
							D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, &featureLevel, &m_pImmediateContext );
		if( SUCCEEDED( hr ) ) break;
	}
	if (FAILED(hr))
	{
		printf("- Create DX Device and swap chain failed.\n");
		return false;
	}

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width  = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f; vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0; vp.TopLeftY = 0;
	m_pImmediateContext->RSSetViewports( 1, &vp );
	if(!initGraphics()) return false;

	// load texture and upate image size.
	if (!loadTextureAndCheckFomart(L"data/fiesta.bmp", &m_srcTexture))
	{
		return false;
	}
	// Create the Const Buffer to transfer const parameters into shader.
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
	hr = m_pd3dDevice->CreateBuffer(&descConstBuffer, &InitData, &m_GPUConstBuffer); // create const buffer.
	if (FAILED(hr))
	{
		printf("-  Create Constant Buffer failed. \n");
		return false;
	}
	m_pImmediateContext->CSSetConstantBuffers(0, 1, &m_GPUConstBuffer);

	if(!createInputBuffer()) return false;
	if(!createOutputBuffer()) return false;
	if(!runComputeShader( L"data/Desaturate.hlsl")) return false;
	return true;
}

/**
*	Run a compute shader loaded from disk shader file. 
*/
bool DXApplication::runComputeShader( LPCWSTR shaderFilename ) 
{
	// Some service variables
	ID3D11UnorderedAccessView *ppUAViewNULL[1] = { NULL };
	ID3D11ShaderResourceView  *ppSRVNULL[2] = { NULL, NULL };

	// Load and compile the shader and create a compute shader DX object. If failed, return.
	if(!loadComputeShader( shaderFilename, &m_computeShader ))
		return false;
	
	m_pImmediateContext->CSSetShader( m_computeShader, NULL, 0 );// Set up the shader and run
	m_pImmediateContext->CSSetShaderResources( 0, 1, &m_srcDataGPUBufferView );// Set Compute Shader Input.
	m_pImmediateContext->CSSetUnorderedAccessViews( 0, 1, &m_destDataGPUBufferView, NULL );// Set Compute Shader Output
	
	// So Dispatch returns immediately ?
	m_pImmediateContext->Dispatch( 60, 60, 1 );// Run Compute Shader

	m_pImmediateContext->CSSetShader( NULL, NULL, 0 );
	m_pImmediateContext->CSSetUnorderedAccessViews( 0, 1, ppUAViewNULL, NULL );
	m_pImmediateContext->CSSetShaderResources( 0, 2, ppSRVNULL );

	byte* gpuDestBufferCopy = getCPUCopyOfGPUDestBuffer(); // get compute result's CPU buffer.

	// Create dest texture from src texture's description.
	// Copy compute shader result into the texture for rendering.
	if (!m_destTexture)
	{
		D3D11_TEXTURE2D_DESC desc;
		m_srcTexture->GetDesc(&desc);
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.MipLevels = 1;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		if (m_pd3dDevice->CreateTexture2D(&desc, NULL, &m_destTexture) != S_OK) return false;
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	// Map destTexture into context.(get a pointer to data in gpu and denies gpu's access)
	// [resources, subresource id,specify cpu's write and read permissions for resource, specify what cpu does when gpu is busy, ]
	if (m_pImmediateContext->Map(m_destTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource) != S_OK)
	{
		printf("- Map dst texture into CPU memory, failed.\n");
		return false;
	}
	memcpy(mappedResource.pData, gpuDestBufferCopy, m_textureDataSize);
	m_pImmediateContext->Unmap(m_destTexture, 0);// Unmap destTexture from Context.

	// Create a view of the output texture
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc; 
	ZeroMemory(&viewDesc, sizeof(viewDesc));
	viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 
	viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MipLevels = 1;
	viewDesc.Texture2D.MostDetailedMip = 0;
	if( FAILED( m_pd3dDevice->CreateShaderResourceView( m_destTexture, &viewDesc, &m_destTextureView) ) )
	{	
		printf( "- Failed to create pixel shader texture resource view.\n" );
		return false;
	}
	return true;
}

/**
*	Update
*/
void DXApplication::update() 
{
}

/**
*	Render the scene
*/
void DXApplication::render() 
{
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pImmediateContext->ClearRenderTargetView( m_pRenderTargetView, ClearColor );

	m_pImmediateContext->PSSetShaderResources( 0, 1, &m_srcTextureView );
	m_pImmediateContext->PSSetShaderResources( 1, 1, &m_destTextureView );
	// draw non-indexed non-instanced primitives.[vertex count, vertex offset in vertex buffer]
	m_pImmediateContext->Draw( 4, 0 );
	m_pSwapChain->Present( 0, 0 );
}

/**
*	Release all the DX resources we have allocated
*/
void DXApplication::release() 
{
	if(m_srcTextureData) delete [] m_srcTextureData;
	m_srcTextureData = NULL;

	SafeRelease(&m_computeShader);

	SafeRelease(&m_destTexture);
	SafeRelease(&m_destTextureView);
	SafeRelease(&m_destDataGPUBuffer);
	SafeRelease(&m_destDataGPUBufferView);

	SafeRelease(&m_srcTexture);
	SafeRelease(&m_srcTextureView);
	SafeRelease(&m_srcDataGPUBuffer);
	SafeRelease(&m_srcDataGPUBufferView);

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
bool DXApplication::initGraphics()
{
	HRESULT hr;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined( DEBUG ) || defined( _DEBUG )
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
	
	ID3DBlob* pErrorBlob;
	ID3DBlob* pVSBlob = NULL;

	// Compile the vertex shader from file.
	if( FAILED(D3DCompileFromFile(L"./data/fullQuad.fx", NULL, NULL, "VS", "vs_4_0", dwShaderFlags, 0, &pVSBlob, &pErrorBlob) ) )
	{
		if( pErrorBlob )
		{
			OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
			pErrorBlob->Release();
		}
		return false;
	}
	if( pErrorBlob ) pErrorBlob->Release();
	// Create the vertex shader
	hr = m_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVertexShader );
	if( FAILED( hr ) )
	{	
		printf( "- Failed to create vertex shader.\n" );
		return false;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	if( FAILED(D3DCompileFromFile(L"./data/fullQuad.fx", NULL, NULL, "PS", "ps_4_0", dwShaderFlags, 0, &pPSBlob, &pErrorBlob) ) )
	{
		if( pErrorBlob )
		{
			OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
			pErrorBlob->Release();
		}
		return false;
	}
	// Create the pixel shader
	hr = m_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShader );
	pPSBlob->Release();
	if( FAILED( hr ) )
	{	
		printf( "- Failed to create pixel shader. \n" );
		return false;
	}

	struct SimpleVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Tex;
	};

	// Create vertex buffer
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
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER; //specify to bind the buffer to input-assembler stage.
	bd.CPUAccessFlags = 0;
	hr = m_pd3dDevice->CreateBuffer( &bd, &InitData, &m_pVertexBuffer );
	if( FAILED( hr ) )
	{	
		printf( "- Failed to create vertex buffer.\n" );
		return false;
	}

	// Define the input layout
	// [semantic name, semantic index for elements with semantic name, data type of element data, input assembler index, offset between elements, input slot class, number of instance to draw]
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	// Create an input-layout object to describe the input-buffer data for the input-assembler stage.
	// [layout description array, number of input data type, compiled shader, size of compiled shader, output]
	hr = m_pd3dDevice->CreateInputLayout(layout, 2, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr))
	{
		printf("- Failed to create input layout.\n");
		return false;
	}

	// Create a render target view from swap chain's back buffer.
	// access one of swap chain's back buffer.[0-based buffer index, interface type which manipulates buffer, output param]
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
	if (FAILED(hr))
	{
		printf("- Cet Back buffer from swap chain failed.\n");
		return false;
	}
	hr = m_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &m_pRenderTargetView );
	pBackBuffer->Release();
	if (FAILED(hr))
	{
		printf("- Create render target from Back buffer failed.\n");
		return false;
	}

	// Create the sample state
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
		printf("- Failed to create texture sampler.\n");
		return false;
	}

	UINT offset = 0, stride = sizeof( SimpleVertex );
	// Bind vertex buffer to input-assembler stage.
	// [start slot index, number of vertex buffer, vertex buffer array, stride array for each vertex buffer, offset array]
	m_pImmediateContext->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &stride, &offset );
	m_pImmediateContext->IASetInputLayout( m_pVertexLayout ); // Set the input layout
	m_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP ); // Set primitive topology

	m_pImmediateContext->PSSetSamplers( 0, 1, &m_pSamplerLinear );
	m_pImmediateContext->VSSetShader( m_pVertexShader, NULL, 0 );
	m_pImmediateContext->PSSetShader( m_pPixelShader, NULL, 0 );
	m_pImmediateContext->OMSetRenderTargets( 1, &m_pRenderTargetView, NULL ); // setup render target into context.
	return true;
}



/**
*	Load a texture from disc and set it so that we can extract the data into a buffer for the compute shader to use.
*   To make everything more clear we also save a copy of the texture data in main memory. We will copy from this buffer
*	the data that we need to feed into the GPU for the compute shader.
*/
bool DXApplication::loadTextureAndCheckFomart(LPCWSTR filename, ID3D11Texture2D** texture)
{
	//https://github.com/Microsoft/DirectXTK/wiki/WICTextureLoader
	if (SUCCEEDED(CreateWICTextureFromFile(m_pd3dDevice, filename, (ID3D11Resource **)texture, &m_srcTextureView)))
	{
		//https://msdn.microsoft.com/en-us/library/windows/desktop/ff476519(v=vs.85).aspx
		// To keep it simple, limit the textures we only load to RGBA 8bits per channel
		// So check the image file format here.
		D3D11_TEXTURE2D_DESC desc;
		(*texture)->GetDesc(&desc);
		if (desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM)
		{
			printf("- Texture format is not qualified.\n");
			return false;
		}
		// update image size. 
		m_imageWidth  = desc.Width;
		m_imageHeight = desc.Height;
		printf("- Texture loaded, size=%dx%d.\n", m_imageWidth, m_imageHeight);
		return true;
	}
	else
	{
		printf("- Texture load failed.\n");
		return false;
	}
}

/**
  *	Once we have the texture data in RAM we create a GPU buffer to feed the compute shader.
  */
bool DXApplication::createInputBuffer()
{
	D3D11_TEXTURE2D_DESC desc;
	m_srcTexture->GetDesc(&desc);
	desc.Usage = D3D11_USAGE_STAGING; // support copy data from GPU to CPU
	desc.BindFlags = 0; desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	ID3D11Texture2D* tempInputTexture;
	if(m_pd3dDevice->CreateTexture2D(&desc, NULL, &tempInputTexture) != S_OK) return false;
	m_pImmediateContext->CopyResource(tempInputTexture, m_srcTexture); // copy resource by GPU.

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if(m_pImmediateContext->Map(tempInputTexture, 0, D3D11_MAP_READ, 0, &mappedResource) != S_OK) return false;
	m_textureDataSize =  mappedResource.RowPitch * desc.Height;
	if(m_srcTextureData) delete [] m_srcTextureData;
	m_srcTextureData = new byte[m_textureDataSize];
	memcpy(m_srcTextureData, mappedResource.pData, m_textureDataSize);
	m_pImmediateContext->Unmap(tempInputTexture, 0);

	// Create buffer in GPU memory
	D3D11_BUFFER_DESC descGPUBuffer;
	ZeroMemory( &descGPUBuffer, sizeof(descGPUBuffer) );
	// https://msdn.microsoft.com/zh-CN/library/ff476085(v=vs.85).aspx
	// Identify how to bind the resources to render pipeline.
	descGPUBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	descGPUBuffer.ByteWidth = m_textureDataSize;
	descGPUBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	descGPUBuffer.StructureByteStride = 4;	// We assume the data is in the RGBA format, 8 bits per channel

	// GPU buffer initial data.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = m_srcTextureData;
	if(FAILED(m_pd3dDevice->CreateBuffer( &descGPUBuffer, &InitData, &m_srcDataGPUBuffer ))) return false;

	// Create shader resource view.
	// [This view is a raw buffer](https://msdn.microsoft.com/ZH-CN/library/ff728736.aspx)
	// https://msdn.microsoft.com/zh-cn/library/ff476900.aspx#Raw_Buffer_Views
	D3D11_SHADER_RESOURCE_VIEW_DESC descView;
	ZeroMemory( &descView, sizeof(descView) );
	descView.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	descView.BufferEx.FirstElement = 0;
	descView.Format = DXGI_FORMAT_UNKNOWN;
	descView.BufferEx.NumElements = descGPUBuffer.ByteWidth / descGPUBuffer.StructureByteStride;
	if (FAILED(m_pd3dDevice->CreateShaderResourceView(m_srcDataGPUBuffer, &descView, &m_srcDataGPUBufferView)))
	{
		printf("- Create Shader Resource View failed.\n");
		return false;
	}
	return true;
}

/**
*	We know the compute shader will output on a buffer which is as big as the texture. Therefore we need to create a
*	GPU buffer and an unordered resource view.
*/
bool DXApplication::createOutputBuffer()
{
	// The compute shader needs to output to some buffer so create a GPU buffer for that.
	D3D11_BUFFER_DESC descGPUBuffer;
	ZeroMemory( &descGPUBuffer, sizeof(descGPUBuffer) );
	descGPUBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	descGPUBuffer.ByteWidth = m_textureDataSize;
	descGPUBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	descGPUBuffer.StructureByteStride = 4;	// We assume the output data is in the RGBA format, 8 bits per channel
	if (FAILED(m_pd3dDevice->CreateBuffer(&descGPUBuffer, NULL, &m_destDataGPUBuffer)))
	{
		printf("- Create Compute Shader output buffer failed.");
		return false;
	}

	// The view we need for the output is an unordered access view. This is to allow the compute shader to write anywhere in the buffer.
	D3D11_UNORDERED_ACCESS_VIEW_DESC descView;
	ZeroMemory( &descView, sizeof(descView) );
	descView.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	descView.Buffer.FirstElement = 0;
	descView.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
	descView.Buffer.NumElements = descGPUBuffer.ByteWidth / descGPUBuffer.StructureByteStride; 
	if(FAILED(m_pd3dDevice->CreateUnorderedAccessView( m_destDataGPUBuffer, &descView, &m_destDataGPUBufferView )))
	{
		printf("- Create Compute Shader output buffer view failed.");
		return false;
	}
	return true;
}

/**
*	Load a compute shader from the specified file always using CSMain as entry point
*/
bool DXApplication::loadComputeShader(LPCWSTR filename, ID3D11ComputeShader** computeShader)
{
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined( _DEBUG )
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	LPCSTR pTarget = ( m_pd3dDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0 ) ? "cs_5_0" : "cs_4_0";
	ID3DBlob* pErrorBlob = NULL;
	ID3DBlob* pBlob = NULL;
	HRESULT hr = D3DCompileFromFile( filename, NULL, NULL, "CSMain", pTarget, dwShaderFlags, NULL, &pBlob, &pErrorBlob);
	if ( FAILED(hr) )
	{
		if ( pErrorBlob ) OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
		if(pErrorBlob) pErrorBlob->Release();
		if(pBlob) pBlob->Release();
		printf("- Create compute shader from file failed.\n");
		return false;
	}
	else
	{
		hr = m_pd3dDevice->CreateComputeShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, computeShader );
		if(pErrorBlob) pErrorBlob->Release();
		if(pBlob) pBlob->Release();
		return hr == S_OK;
	}
}

/**
 *	Get a copy of the GPU dest buffer.
 *	// Resources usage. https://msdn.microsoft.com/en-us/library/windows/desktop/ff476259(v=vs.85).aspx
 */
byte* DXApplication::getCPUCopyOfGPUDestBuffer()
{
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
}