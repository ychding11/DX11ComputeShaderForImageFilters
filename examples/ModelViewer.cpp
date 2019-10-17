//=================================================================================================
// ModelViewer example
//   - Load assimp Model,shading without lighting.
//   - Maybe extend to more complicated shading algorithm.
//
//  All code licensed under the MIT license
//
//=================================================================================================


#pragma once

#include <vector> 
#include "imgui.h"
#include "App.h"

#include "GHIResources.h"
#include "GHICommandContext.h"

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
        shaderProgram = &depthOnly;
        //shaderProgram = &noLighting;
		shaderProgram->Init(commandContext);

		model3d.CreateWithAssimp(commandContext, modelPath.c_str()); // load model 
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
	{ }
	
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
	{ }
};


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    ModelViewer viewer;
    viewer.Run();
    return 0;
}
