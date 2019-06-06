#include <io.h>
#include <iostream>

#include "EffectManager.h"
#include "Logger.h"
#include "Utils.h"

void EffectManager::CheckEffect()
{
    for (auto it = mEffects.begin(); it != mEffects.end(); ++it)
        Logger::getLogger() << it->first << " <===> " << it->second << '\n';
}

EffectPtr EffectManager::NextEffect(std::string &name)
{
    auto ret =  mCurrentEffect == mEffects.end() ? --mCurrentEffect : mCurrentEffect++;
	Info("- Next Effect: %s\n", ret->first.c_str());
    name = ret->first;
    return ret->second;

}
EffectPtr EffectManager::PrevEffect(std::string &name)
{
    auto ret =  mCurrentEffect == mEffects.begin() ? mCurrentEffect : --mCurrentEffect;
	Info("- Prev Effect: %s\n", ret->first.c_str());
    name = ret->first;
    return ret->second;
}

int EffectManager::LoadEffectFileList(std::string dir)
{
    getFiles(dir, mFileList);
    return mFileList.size();
}


void EffectManager::ClearEffects(void)
{
    for (auto it = mEffects.begin(); it != mEffects.end(); ++it)
    {
        if (it->second) it->second->Release();
    }
    mEffects.clear();
    mCurrentEffect = mEffects.end();
    assert(mEffects.size() == 0);
}

void EffectManager::BuildEffects()
{
    int n = LoadEffectFileList(EFFECT_REPO);
    ClearEffects();
    for (int i = 0; i < n; ++i)
    {
        std::string filename = mFileList[i];
        LPCWSTR wfilename = CharPtrToLPCWSTR(filename.c_str());
        EffectPtr ptrEffect = NULL;
		Info("- Begin build Compute Shader: %s\n", filename.c_str());
        bool build = LoadComputeShader(wfilename, "CSMain", &ptrEffect);
        delete[] wfilename;
		if (build == true)
		{
			Info("- Build Compute Shader OK\n");
		}
		else
		{
			exit(1);
		}

        auto ret = mEffects.insert( std::pair<std::string, EffectPtr>(filename, ptrEffect) );
        if (ret.second == false)
        {
			Info("- Effect file exist:", filename.c_str());
        }
    }
    mCurrentEffect = mEffects.begin();
}

/*
*	Load a compute shader from  file and use CSMain as entry point
*/
bool EffectManager::LoadComputeShader(LPCWSTR filename, LPCSTR entrypoint, ID3D11ComputeShader** computeShader)
{
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined( _DEBUG )
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob = NULL;
    ID3DBlob* pBlob = NULL;
    LPCSTR pTarget = (mpd3dDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";
    HRESULT hr = D3DCompileFromFile(filename, NULL, NULL, entrypoint, pTarget, dwShaderFlags, NULL, &pBlob, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob) OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
        if (pErrorBlob) pErrorBlob->Release();
        if (pBlob) pBlob->Release();
		Error("- Compile Compute Shader Failed.");
		return false;
    }
    else
    {
        hr = mpd3dDevice->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, computeShader);
        if (pErrorBlob) pErrorBlob->Release();
        if (pBlob) pBlob->Release();
		return true;
    }
}