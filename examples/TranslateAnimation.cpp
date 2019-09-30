#pragma once


#include <vector> 
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

class TranslateAnimation:public GHI::App
{
private:
    GHI::Mesh  testMesh;
    GHI::FirstPersonCamera camera;
    GHI::MouseState mouseState;

    GHI::GHIShaderProgram *shaderProgram = nullptr;
    GHI::DepthOnlyShader  depthOnly;
    GHI::NoLightingShader noLighting;
    GHI::RawInstancedShader rawInstance;

    GHI::GHIBuffer *instanceBuffer = nullptr;

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
        //shaderProgram = &rawInstance;
        shaderProgram->Init(commandContext);


        GHI::Float3 dimension(1.0, 1.0, 1.0);
        GHI::Float3 position(.0, .0, .0);
        GHI::Quaternion orientation;

        testMesh.InitBox(commandContext, dimension, position, orientation, 0);
        GHI::Float3 pos = testMesh.boundingbox.Centroid() + testMesh.boundingbox.DiagnalLen() * GHI::Float3(0, 0, -1);
        camera.SetPosition(pos);

        GHI::Float4x4 instanceData[2];
        instanceData[0].SetTranslation(GHI::Float3(-1.0, 0, 0));
        instanceData[1].SetTranslation(GHI::Float3( 1.0, 0, 0));

        // NOT as a shader input, no need to transpose
        //instanceData[0] = GHI::Float4x4::Transpose(instanceData[0]);
        //instanceData[1] = GHI::Float4x4::Transpose(instanceData[1]);

        rawInstance.instanceBuffer = commandContext->CreateVertexBuffer(sizeof(GHI::Float4x4) * 2, instanceData);
        rawInstance.instanceCount  = 2;
        rawInstance.instanceOffset = 0;
        rawInstance.instanceStride = 64;
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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    TranslateAnimation viewer;
    viewer.Run();
    return 0;
}