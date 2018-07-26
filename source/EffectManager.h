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
    std::map<std::string, EffectPtr>::iterator  mCurrentEffect;
    ID3D11Device* mpd3dDevice;

    EffectManager(ID3D11Device* device)
        : mpd3dDevice(device)
    {
        BuildEffects();
    }

public:
    
    ~EffectManager()
    {
        ClearEffects();
    }

    static EffectManager* GetEffectManager(ID3D11Device* device)
    {
        static EffectManager* mgr = new EffectManager(device);
        return mgr;
    }

    void BuildEffects();

    void CheckEffect();

    EffectPtr NextEffect();

    EffectPtr PrevEffect();

    std::string CurrentEffectName() const
    {
        if (mCurrentEffect != mEffects.end())
            return mCurrentEffect->first;
        else
            return "None Effect";
    }

private:
    /*
     *	Load a compute shader from  file and use CSMain as entry point
     */
    void LoadComputeShader(LPCWSTR filename, LPCSTR entrypoint, ID3D11ComputeShader** computeShader);

    int LoadEffectFileList(std::string dir);

    void ClearEffects(void);

    
};