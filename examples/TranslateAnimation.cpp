//=================================================================================================
// Translate Animation example
//   - Translate x coordinate of a box to make a simple animation.
//
//  All code licensed under the MIT license
//
//=================================================================================================


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
    GHI::NoLightingShader noLighting;

    const float maxTranslateX;
    float animatedX = 0.f;
    float animatedSpeed = 0.05f;

public:

    TranslateAnimation()
        : App(L"TranslateAnimation")
        , camera(16.0f / 9.0f, Pi_4 * 0.75f, NearClip, FarClip)
        , maxTranslateX(3.f)
    {
        window.RegisterMessageCallback(WindowMessageCallback, this);
    }
protected:

    virtual void Initialize() override
    {
        shaderProgram = &noLighting;
        shaderProgram->Init(commandContext);

        GHI::Float3 dimension(1.0, 1.0, 1.0);
        GHI::Float3 position(.0, .0, .0);
        GHI::Quaternion orientation;

        testMesh.InitBox(commandContext, dimension, position, orientation, 0); // create a box 
        GHI::Float3 pos = testMesh.boundingbox.Centroid() + testMesh.boundingbox.DiagnalLen() * GHI::Float3(0, 0, -1);
        camera.SetPosition(pos);
    }

    virtual void Update(const GHI::Timer& timer) override
    {
        // set shader parameter
        GHI::UserData userdata;
        userdata.camera = &camera;
        userdata.worldMat.SetTranslation(GHI::Float3(animatedX, 0, 0));
        shaderProgram->Update(userdata, commandContext);

        // update "animated X coordinate" by a frame delta.
        float deltTime = timer.DeltaSecondsF();
        deltTime < 1e-3f ? deltTime = 1e-3f : deltTime;
        animatedX += deltTime * animatedSpeed;
        animatedX > maxTranslateX ? animatedX = -maxTranslateX : animatedX;
    }

    virtual void Render(const GHI::Timer& timer) override
    {
        // bind shaders & drawing
        shaderProgram->Apply(testMesh, commandContext);
    }

    virtual void Shutdown() override
    { }

private:
    //// Windows Message handler
    static void WindowMessageCallback(void* context, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    { }
};

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    TranslateAnimation viewer;
    viewer.Run();
    return 0;
}