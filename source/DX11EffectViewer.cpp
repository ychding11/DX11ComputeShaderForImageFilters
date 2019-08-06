#include "DX11EffectViewer.h"
#include "Utils.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define IMAGE_REPO "..\\images"

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

int	DX11EffectViewer::initialize()
{
	loadImage(m_defaultImage); //< Load source image as texture and upate image size.
	mFinalTexture = commandContext->CreateTextureByAnother(mSrcTexture);
	mDstTexture = commandContext->CreateTextureByAnother(mSrcTexture);

    getFiles(IMAGE_REPO , mImageList);
    mCurrentImage = mImageList.begin();

	INFO("DX11EffectViewer Initialized, default image:%s\n", m_defaultImage.c_str());
	return 0;
}

void DX11EffectViewer::render() 
{
    if (mDisplayMode == DisplayMode::ONLY_RESULT)
    {
        DrawFullScreenTriangle({0.f, 0.f, m_imageWidth+0.f, m_imageHeight+0.f}, mFinalTexture);
    }
    else if (mDisplayMode == DisplayMode::ONLY_SOURCE)
    {
        DrawFullScreenTriangle({0.f, 0.f, m_imageWidth+0.f, m_imageHeight+0.f}, mSrcTexture);
    }
    else if (mDisplayMode == DisplayMode::SOURCE_RESULT)
    {
	    float width, height;
	    width = float(SwapchainWidth() - 2) / 2.f;
	    height = 1.f / m_Aspect * width;

        DrawFullScreenTriangle({0.f, 0.f, width, height}, mSrcTexture);
        DrawFullScreenTriangle({width + 2.f, 0.f, width, height}, mFinalTexture);
    }
    else
    {
    }
}

void DX11EffectViewer::Shutdown() 
{
}

#if 0
void DX11EffectViewer::UpdateCSConstBuffer()
{
	CB data;
	data.iHeight = m_imageHeight;
	data.iWidth = m_imageWidth;
	commandContext->UpdateBuffer(mConstBuffer,&data, sizeof(data));
}

bool DX11EffectViewer::CreateCSConstBuffer()
{
	CB cb = { m_imageWidth, m_imageHeight };
	mConstBuffer = commandContext->CreateConstBuffer(sizeof(cb), &cb);
	commandContext->SetConstBuffer(mConstBuffer, 0);
	return true;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////
//////
////// https://github.com/Microsoft/DirectXTK/wiki/WICTextureLoader
//////
//////////////////////////////////////////////////////////////////////////////////////////////////
bool DX11EffectViewer::loadImage(std::string imagefile)
{
	mSrcTexture = commandContext->CreateTexture(imagefile);
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
				app->NextEffect();
			}
			else if (wParam == VK_F2)
			{
				app->PrevEffect();
			}
			else if (wParam == VK_F3)
			{
				app->NextImage();
			}
			else if (wParam == VK_ESCAPE)
			{
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
			else if (key == 'u')
			{
				app->UpdateEffects();
				app->NextEffect();
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