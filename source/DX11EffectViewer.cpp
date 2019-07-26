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
		*ppT = nullptr;
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

	LoadImageAsSrcTexture(); //< Load source image as texture and upate image size.
	mFinalTexture = commandContext->CreateTextureByAnother(mSrcTexture);
	mDstTexture = commandContext->CreateTextureByAnother(mSrcTexture);

	CreateCSConstBuffer(m_pd3dDevice);

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

	//SimpleFramework::GHIUAVParam uav;
	commandContext->SetShaderResource(mDstTexture, 0, SimpleFramework::GHIUAVParam());
	commandContext->SetShaderResource(mSrcTexture, 0, SimpleFramework::GHISRVParam());
    commandContext->SetSampler(linearSampler, 0, SimpleFramework::EShaderStage::CS);
	commandContext->SetConstBuffer(mConstBuffer, 0);
	commandContext->Dispatch( (m_imageWidth + 31) / 32, (m_imageHeight + 31) / 32, 1 );

	m_pImmediateContext->CSSetShader( NULL, NULL, 0 );
	m_pImmediateContext->CSSetUnorderedAccessViews( 0, 1, ppUAViewNULL, NULL );
	m_pImmediateContext->CSSetShaderResources( 0, 1, ppSRVNULL );

    //m_pImmediateContext->CopyResource(m_resultImageTexture, tempCSOutputTexture); //< dst <-- src
    commandContext->CopyTexture(mFinalTexture, mDstTexture); //< dst <-- src
}

void DX11EffectViewer::Render() 
{
	m_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
	m_pImmediateContext->VSSetShader( m_pVertexShader, NULL, 0 );
    commandContext->SetSampler(linearSampler, 0, SimpleFramework::EShaderStage::PS);

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
        commandContext->SetShaderResource(mSrcTexture,0,SimpleFramework::GHISRVParam(),SimpleFramework::EShaderStage::PS);
        m_pImmediateContext->PSSetShader( m_pPixelShaderSrcImage, NULL, 0 );
        DrawFullScreenTriangle({0.f, 0.f, m_imageWidth+0.f, m_imageHeight+0.f});

        commandContext->SetShaderResource(mFinalTexture,1,SimpleFramework::GHISRVParam(),SimpleFramework::EShaderStage::PS);
        m_pImmediateContext->PSSetShader( m_pPixelShaderResultImage, NULL, 0 );
        DrawFullScreenTriangle({m_imageWidth + 1.f, 0.f, m_imageWidth+0.f, m_imageHeight+0.f});
    }
}

void DX11EffectViewer::RenderMultiViewport()
{
	float width, height;
	width = float(SwapchainWidth() - 2) / 2.;
	height = 1.f / m_Aspect * width;

	m_pImmediateContext->PSSetShader( m_pPixelShaderSrcImage, NULL, 0 );
    commandContext->SetShaderResource(mSrcTexture,0,SimpleFramework::GHISRVParam(), SimpleFramework::EShaderStage::PS);
    DrawFullScreenTriangle({0.f, 0.f, width, height});

    commandContext->SetShaderResource(mFinalTexture,1,SimpleFramework::GHISRVParam(),SimpleFramework::EShaderStage::PS);
	m_pImmediateContext->PSSetShader( m_pPixelShaderResultImage, NULL, 0 );
    DrawFullScreenTriangle({width + 2.f, 0.f, width, height});
}

void	DX11EffectViewer::RenderSourceImage()
{
    commandContext->SetShaderResource(mSrcTexture,0,SimpleFramework::GHISRVParam(), SimpleFramework::EShaderStage::PS);
	m_pImmediateContext->PSSetShader( m_pPixelShaderSrcImage, NULL, 0 );
    DrawFullScreenTriangle({0.f, 0.f, m_imageWidth+0.f, m_imageHeight+0.f});
}

void	DX11EffectViewer::RenderResultImage()
{
    commandContext->SetShaderResource(mFinalTexture,0,SimpleFramework::GHISRVParam(),SimpleFramework::EShaderStage::PS);
	m_pImmediateContext->PSSetShader( m_pPixelShaderResultImage, NULL, 0 );
    DrawFullScreenTriangle({0.f, 0.f, m_imageWidth+0.f, m_imageHeight+0.f});
}

void DX11EffectViewer::Shutdown() 
{
	SafeRelease(&m_pVertexShader);
	SafeRelease(&m_pPixelShaderSrcImage);
	SafeRelease(&m_pPixelShaderResultImage);
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

	Info("InitGraphics OK. @%s:%d\n", __FILE__, __LINE__);
}

void    DX11EffectViewer::UpdateCSConstBuffer()
{
	CB data;
	data.iHeight = m_imageHeight;
	data.iWidth = m_imageWidth;
	commandContext->UpdateBuffer(mConstBuffer,&data, sizeof(data));
	Info("- Update Constant buffer.\n");
}

bool DX11EffectViewer::CreateCSConstBuffer(ID3D11Device* pd3dDevice)
{
	CB cb = { m_imageWidth, m_imageHeight };
	mConstBuffer = commandContext->CreateConstBuffer(sizeof(cb), &cb);
	commandContext->SetConstBuffer(mConstBuffer, 0);
	Info("- Create Constant buffer and Bind to CS OK.\n");
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//////
////// https://github.com/Microsoft/DirectXTK/wiki/WICTextureLoader
//////
//////////////////////////////////////////////////////////////////////////////////////////////////
bool DX11EffectViewer::LoadImageAsSrcTexture()
{
	mSrcTexture = commandContext->CreateTexture(m_imageName);
	m_imageWidth = mSrcTexture->width;
	m_imageHeight = mSrcTexture->height;
	m_Aspect = mSrcTexture->aspect;
	m_textureSizeInBytes = mSrcTexture->textureSizeInBytes;

	return m_imageWidth > 0 ? true : false;
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
#if 0 
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