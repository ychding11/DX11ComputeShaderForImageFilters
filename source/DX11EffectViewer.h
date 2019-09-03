#pragma once

// code migration. https://msdn.microsoft.com/en-us/library/windows/desktop/ee418730(v=vs.85).aspx
//#include <DirectXMath.h>

#include <vector> 
#include "imgui.h"
#include "ImNodes.h"
#include "ImNodesEz.h"
#include "Filter.h"
#include "Utils.h"
#include "App.h"

#include "GHIResources.h"
#include "GHICommandContext.h"

struct alignas(16) CB
{
	int iWidth;
	int iHeight;
};

enum DisplayMode
{
    ONLY_SOURCE   = 0,
    ONLY_RESULT   = 1,
    SOURCE_RESULT = 2,
    ALL_MODE      = 3,
};

#define SHADERS_REPO "..\\effects"

class DX11EffectViewer : public GHI::App
{

public:
    //DisplayMode mDisplayMode = DisplayMode::ONLY_SOURCE;
    DisplayMode mDisplayMode = DisplayMode::SOURCE_RESULT;
	std::string m_defaultImage;

	DX11EffectViewer() 
		: App(L"Filters")
		, m_imageWidth(0)
		, m_imageHeight(0)
        , m_defaultImage("../images/test.png")
	{
		window.RegisterMessageCallback(WindowMessageCallback,this);
	}

	void SaveResult();
	
    void NextEffect()
    {
		mCurFilter+1 == mFilters.end() ? mCurFilter = mFilters.begin() : mCurFilter++;
        activeCurFilter();
    }

    void PrevEffect()
    {
		mCurFilter == mFilters.begin() ? mCurFilter = mFilters.end()-1 : mCurFilter--;
        activeCurFilter();
    }

    void NextImage()
    {
        mCurrentImage+1 == mImageList.end() ? mCurrentImage = mImageList.begin() : mCurrentImage++;
		DEBUG("Switch to image [%s]\n", (*mCurrentImage).c_str());

		while (!loadImage((*mCurrentImage)))
		{
			DEBUG("image [%s] load failed.\n", (*mCurrentImage).c_str());
			mCurrentImage+1 == mImageList.end() ? mCurrentImage = mImageList.begin() : mCurrentImage++;
			DEBUG("Switch to image [%s]\n", (*mCurrentImage).c_str());
		}
	    mDstTexture = commandContext->CreateTextureByAnother(mSrcTexture);
	    mFinalTexture = commandContext->CreateTextureByAnother(mSrcTexture);

        activeCurFilter();
    }

    void UpdateEffects()
    { }

    int     imageHeight() const { return m_imageHeight; }
    int     imageWidth()  const { return m_imageWidth; }

protected:

	virtual void Initialize() override
	{
		initialize();
		std::vector<std::string> files;
		getFiles(SHADERS_REPO, files);

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

		mCurFilter = mFilters.begin();
        activeCurFilter();
	}

	virtual void Update(const GHI::Timer& timer) override
	{
		this->updateUI();
		(*mCurFilter)->UpdateUI(commandContext);
	}
	virtual void Render(const GHI::Timer& timer) override
	{
		render();
	}

    virtual void Shutdown() override;

	static void WindowMessageCallback(void* context, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:

	void	render();
	int     initialize();

    void activeCurFilter()
    {
		(*mCurFilter)->addInput(mSrcTexture);
		(*mCurFilter)->addOutput(mDstTexture);
		(*mCurFilter)->Active(commandContext);
		commandContext->CopyTexture(mFinalTexture, mDstTexture); //< dst <-- src
    }

    std::vector<std::string> mImageList;
    std::vector<std::string>::iterator mCurrentImage;

	bool loadImage(std::string imagefile);
	void updateUI()
	{
		ImGui::Begin("UI");

		ImGui::Text("This is experialment."); // Display some text (you can use a format strings too)
		ImGui::ColorEdit4("clear color", clearColor, ImGuiColorEditFlags_Float); // floats representing a color

		if (ImGui::Button("Save")) // Buttons return true when clicked (most widgets return true when edited/activated)
		{
		}
		ImGui::SameLine();
		ImGui::Text("Application Average: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	//bool CreateCSConstBuffer();
   // void UpdateCSConstBuffer();

	byte*	GetResultImage();
    
	// Fields
	int	m_imageWidth = 0;
	int	m_imageHeight = 0;
	float m_Aspect = 1.f;
	UINT m_textureSizeInBytes;

	GHI::GHIBuffer *mConstBuffer = nullptr;
	GHI::GHITexture *mSrcTexture = nullptr;
	GHI::GHITexture *mDstTexture = nullptr;
	GHI::GHITexture *mFinalTexture = nullptr;
	std::vector<Filter*> mFilters;
	std::vector<Filter*>::iterator mCurFilter;
};


#include <io/Model.h> 
#include "GHIShaderProgram.h"


static const float NearClip = 0.01f;
static const float FarClip = 100.0f;

// Constants
const float Pi = 3.141592654f;
const float Pi2 = 6.283185307f;
const float Pi_2 = 1.570796327f;
const float Pi_4 = 0.7853981635f;
const float InvPi = 0.318309886f;
const float InvPi2 = 0.159154943f;

class ModelViewer :public GHI::App
{
private:
	std::string modelPath = "..\\Content\\Models\\Box\\Box_Lightmap.fbx";
	GHI::Model model3d;
	GHI::FirstPersonCamera camera;
    GHI::MouseState mouseState;

    GHI::GHIShaderProgram *shaderProgram = nullptr;
	GHI::DepthOnlyShader  depthOnly;
	GHI::NoLightingShader noLighting;

public:

	ModelViewer()
		: App(L"ModelViewerWithoutTexture")
        , camera(16.0f / 9.0f, Pi_4 * 0.75f, NearClip, FarClip)

	{
		window.RegisterMessageCallback(WindowMessageCallback, this);
	}
protected:

	virtual void Initialize() override
	{
        //shaderProgram = &depthOnly;
        shaderProgram = &noLighting;
		shaderProgram->Init(commandContext);

		model3d.CreateWithAssimp(commandContext, modelPath.c_str());
		const GHI::Mesh& mesh = const_cast<const GHI::Mesh&>(model3d.Meshes()[0]);
		GHI::Float3 pos = mesh.boundingbox.Centroid() + mesh.boundingbox.DiagnalLen() * GHI::Float3(0, 0, -1);

        camera.SetPosition(pos);
	}

	virtual void Update(const GHI::Timer& timer) override
	{
        GHI::UserData userdata;
        userdata.camera = &camera;
        userdata.worldMat.SetTranslation(GHI::Float3(0,0,0));

		updateInput(timer);
		shaderProgram->Update(userdata, commandContext);
	}

	virtual void Render(const GHI::Timer& timer) override
	{
		//for (int i = 0; i < model3d.Meshes().size(); ++i)
			shaderProgram->Apply(const_cast<const GHI::Mesh&>(model3d.Meshes()[0]), commandContext);
	}

	virtual void Shutdown() override
	{

	}
	
private:
	void updateInput(const GHI::Timer& timer)
    {
        using namespace GHI;

        mouseState = MouseState::GetMouseState(window);
        KeyboardState kbState = KeyboardState::GetKeyboardState(window);

        if (kbState.IsKeyDown(KeyboardState::Escape))
            window.Destroy();

              float CamMoveSpeed = 5.0f   * timer.DeltaSecondsF();
        const float CamRotSpeed  = 0.180f * timer.DeltaSecondsF();
        const float MeshRotSpeed = 0.180f * timer.DeltaSecondsF();

        // Move the camera with keyboard input
        if (kbState.IsKeyDown(KeyboardState::LeftShift))
            CamMoveSpeed *= 0.25f;

        Float3 camPos = camera.Position();
        if (kbState.IsKeyDown(KeyboardState::W))
            camPos += camera.Forward() * CamMoveSpeed;
        else if (kbState.IsKeyDown(KeyboardState::S))
            camPos += camera.Back() * CamMoveSpeed;
        if (kbState.IsKeyDown(KeyboardState::A))
            camPos += camera.Left() * CamMoveSpeed;
        else if (kbState.IsKeyDown(KeyboardState::D))
            camPos += camera.Right() * CamMoveSpeed;
        if (kbState.IsKeyDown(KeyboardState::Q))
            camPos += camera.Up() * CamMoveSpeed;
        else if (kbState.IsKeyDown(KeyboardState::E))
            camPos += camera.Down() * CamMoveSpeed;
        camera.SetPosition(camPos);

        // Rotate the camera by the mouse
        if (mouseState.RButton.Pressed && mouseState.IsOverWindow)
        {
            float xRot = camera.XRotation();
            float yRot = camera.YRotation();
            xRot += mouseState.DY * CamRotSpeed;
            yRot += mouseState.DX * CamRotSpeed;
            camera.SetXRotation(xRot);
            camera.SetYRotation(yRot);
        }

        //camera.SetFieldOfView(AppSettings::VerticalFOV(camera.AspectRatio()));
	}

    //// Windows Message handler
	static void WindowMessageCallback(void* context, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{

	}
};

/*===============================================================*/
class TranslateAnimation:public GHI::App
{
private:
    GHI::Mesh  testMesh;
    GHI::FirstPersonCamera camera;
    GHI::MouseState mouseState;

    GHI::GHIShaderProgram *shaderProgram = nullptr;
    GHI::DepthOnlyShader  depthOnly;
    GHI::NoLightingShader noLighting;

public:

    TranslateAnimation()
        : App(L"TranslateAnimation")
        , camera(16.0f / 9.0f, Pi_4 * 0.75f, NearClip, FarClip)

    {
        window.RegisterMessageCallback(WindowMessageCallback, this);
    }
protected:

    virtual void Initialize() override
    {
        //shaderProgram = &depthOnly;
        shaderProgram = &noLighting;
        shaderProgram->Init(commandContext);


        GHI::Float3 dimension(1.0, 1.0, 1.0);
        GHI::Float3 position(.0, .0, .0);
        GHI::Quaternion orientation;

        testMesh.InitBox(commandContext, dimension, position, orientation, 0);
        GHI::Float3 pos = testMesh.boundingbox.Centroid() + testMesh.boundingbox.DiagnalLen() * GHI::Float3(0, 0, -1);
        camera.SetPosition(pos);
    }

    virtual void Update(const GHI::Timer& timer) override
    {
        const float maxTranslate = 3.f;
        static float x = -maxTranslate;
        //float deltTime = timer.DeltaSeconds();
        float deltTime = 0.01f;
        float speed = 0.05f;

        GHI::UserData userdata;
        userdata.camera = &camera;
        userdata.worldMat.SetTranslation(GHI::Float3(x, 0, 0));
        updateInput(timer);
        shaderProgram->Update(userdata, commandContext);

        x += deltTime * speed;
        x > maxTranslate ? x = -maxTranslate : x;

    }

    virtual void Render(const GHI::Timer& timer) override
    {
        shaderProgram->Apply(testMesh, commandContext);
    }

    virtual void Shutdown() override
    {

    }

private:
    void updateInput(const GHI::Timer& timer)
    {
        using namespace GHI;

        mouseState = MouseState::GetMouseState(window);
        KeyboardState kbState = KeyboardState::GetKeyboardState(window);

        if (kbState.IsKeyDown(KeyboardState::Escape))
            window.Destroy();

        float CamMoveSpeed = 5.0f   * timer.DeltaSecondsF();
        const float CamRotSpeed = 0.180f * timer.DeltaSecondsF();
        const float MeshRotSpeed = 0.180f * timer.DeltaSecondsF();

        // Move the camera with keyboard input
        if (kbState.IsKeyDown(KeyboardState::LeftShift))
            CamMoveSpeed *= 0.25f;

        Float3 camPos = camera.Position();
        if (kbState.IsKeyDown(KeyboardState::W))
            camPos += camera.Forward() * CamMoveSpeed;
        else if (kbState.IsKeyDown(KeyboardState::S))
            camPos += camera.Back() * CamMoveSpeed;
        if (kbState.IsKeyDown(KeyboardState::A))
            camPos += camera.Left() * CamMoveSpeed;
        else if (kbState.IsKeyDown(KeyboardState::D))
            camPos += camera.Right() * CamMoveSpeed;
        if (kbState.IsKeyDown(KeyboardState::Q))
            camPos += camera.Up() * CamMoveSpeed;
        else if (kbState.IsKeyDown(KeyboardState::E))
            camPos += camera.Down() * CamMoveSpeed;
        camera.SetPosition(camPos);

        // Rotate the camera by the mouse
        if (mouseState.RButton.Pressed && mouseState.IsOverWindow)
        {
            float xRot = camera.XRotation();
            float yRot = camera.YRotation();
            xRot += mouseState.DY * CamRotSpeed;
            yRot += mouseState.DX * CamRotSpeed;
            camera.SetXRotation(xRot);
            camera.SetYRotation(yRot);
        }

        //camera.SetFieldOfView(AppSettings::VerticalFOV(camera.AspectRatio()));
    }

    //// Windows Message handler
    static void WindowMessageCallback(void* context, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {

    }
};