#include "EffectViewer.h"
#include "Utils.h"

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

int	EffectViewer::initialize()
{
	return 0;
}

void EffectViewer::Initialize()
{
	initialize();

	for (auto & f : mImageList)
		mTextures[f] = commandContext->CreateTexture(f);

	Filter *filter = nullptr;

#if 0
	for (auto it = files.begin(); it != files.end() && (*it) != "..\\effects\\test.hlsl"; ++it)
	{
		filter = new Filter(*it);
		filter->Init(commandContext);
		filter->setSampler(linearSampler);
		mFilters.push_back(filter);
	}
#endif

	filter = new BilaterialFilter();
	filter->Init(commandContext);
	filter->setSampler(linearSampler);
	mFilters.push_back(filter);

	filter = new FishEyeFilter();
	filter->Init(commandContext);
	filter->setSampler(linearSampler);
	mFilters.push_back(filter);

	filter = new SwirlFilter();
	filter->Init(commandContext);
	filter->setSampler(linearSampler);
	mFilters.push_back(filter);

	filter = new LensCircleFilter();
	filter->Init(commandContext);
	filter->setSampler(linearSampler);
	mFilters.push_back(filter);
}

void EffectViewer::render() 
{
	float width = mFinalTexture->width;
	float height = mFinalTexture->height;
	float aspect = mFinalTexture->aspect;
    if (mDisplayMode == DisplayMode::ONLY_RESULT)
    {
        DrawFullScreenTriangle({0.f, 0.f, width, height}, mFinalTexture);
    }
    else if (mDisplayMode == DisplayMode::ONLY_SOURCE)
    {
        DrawFullScreenTriangle({0.f, 0.f, width, height}, mSrcTexture);
    }
    else if (mDisplayMode == DisplayMode::SOURCE_RESULT)
    {
	    float width, height;
	    width = float(SwapchainWidth() - 2) / 2.f;
	    height = 1.f / aspect * width;

        DrawFullScreenTriangle({0.f, 0.f, width, height}, mSrcTexture);
        DrawFullScreenTriangle({width + 2.f, 0.f, width, height}, mFinalTexture);
    }
    else
    {

    }
}

void EffectViewer::Shutdown() 
{
}

#if 0
void EffectViewer::UpdateCSConstBuffer()
{
	CB data;
	data.iHeight = m_imageHeight;
	data.iWidth = m_imageWidth;
	commandContext->UpdateBuffer(mConstBuffer,&data, sizeof(data));
}

bool EffectViewer::CreateCSConstBuffer()
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

void EffectViewer::SaveResult()
{
}

/**
 *	Get a copy of the GPU dest buffer.
 *	Resources usage. https://msdn.microsoft.com/en-us/library/windows/desktop/ff476259(v=vs.85).aspx
 */
byte* EffectViewer::GetResultImage()
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

void EffectViewer::WindowMessageCallback(void* context, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	EffectViewer *app = (EffectViewer*)context;
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


void EffectViewer::updateUI()
{
	int  mChildWinWidth = 380;
	int  mChildWinHeight = 200;
	bool mChildWinBorder = true;

	std::string images;
	for (auto & s : mImageList) { images.append(s.begin(), s.end()); images.push_back('\0'); };
	std::string effects;
	for (auto & s : mFilters) { effects.append(s->mShaderFile.begin(), s->mShaderFile.end()); effects.push_back('\0'); };

	ImGui::SetNextWindowSize(ImVec2(380, 580), ImGuiCond_FirstUseEver);
	ImGui::Begin("settings", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Close"))  false;
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	//< Why child window title cannot be displayed ?
	ImGui::BeginChild("Scene", ImVec2(mChildWinWidth, mChildWinHeight), mChildWinBorder, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("FPS: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Combo(" Images", &mImageIndex, images.c_str());
	ImGui::Combo(" Effects", &mEffectIndex, effects.c_str());
	ImGui::Separator();

	ImGui::BulletText("Image : %s", mImageList[mImageIndex].c_str());
	ImGui::Separator();
	ImGui::BulletText("Effect: %s", mFilters[mEffectIndex]->mShaderFile.c_str());
	ImGui::EndChild();
	mFilters[mEffectIndex]->UpdateUI(commandContext); //< filter UI
	ImGui::End();

	bool stateChanged = false;
	if (mActiveImageIndex != mImageIndex)
	{
		mActiveImageIndex = mImageIndex;
		stateChanged = true;
		mSrcTexture = mTextures[mImageList[mActiveImageIndex]];

		//< need refine in future time
		mDstTexture = commandContext->CreateTextureByAnother(mSrcTexture);
		mFinalTexture = commandContext->CreateTextureByAnother(mSrcTexture);
	}
	if (mActiveEffectIndex != mEffectIndex)
	{
		mActiveEffectIndex = mEffectIndex;
		stateChanged = true;
	}
	if (stateChanged)
	{
		mFilters[mEffectIndex]->addInput(mSrcTexture);
		mFilters[mEffectIndex]->addOutput(mDstTexture);
		mFilters[mEffectIndex]->Active(commandContext);
		commandContext->CopyTexture(mFinalTexture, mDstTexture); //< dst <-- src
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	EffectViewer viewer;
	viewer.Run();
	return 0;
}