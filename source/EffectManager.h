#pragma once

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#if D3D_COMPILER_VERSION < 46
#include <d3dx11.h>
#endif

#include <map>
#include <vector>
#include <string>

typedef ID3D11ComputeShader*  EffectPtr;

class EffectManager
{
    std::vector<std::string> mFileList;
    std::map<std::string, EffectPtr> mEffects;
    ID3D11Device* mpd3dDevice;

    EffectManager(ID3D11Device* device)
        : mpd3dDevice(device)
    {
        BuildEffects();
    }

public:
    static EffectManager* GetEffectManager(ID3D11Device* device)
    {
        static EffectManager* mgr = NULL;
        if (mgr == NULL) mgr = new EffectManager(device);
        return mgr;
    }

    void BuildEffects();

    EffectPtr* GetEffect();

private:
    /*
     *	Load a compute shader from  file and use CSMain as entry point
     */
    void LoadComputeShader(LPCWSTR filename, LPCSTR entrypoint, ID3D11ComputeShader** computeShader);

    int LoadEffectFileList(std::string dir);

    
};