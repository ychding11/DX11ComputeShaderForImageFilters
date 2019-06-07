#include <io.h>
#include <iostream>

#include "EffectManager.h"
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
        bool build = BuildComputeShader(wfilename, "CSMain", &ptrEffect);
        delete[] wfilename;
		if (build == true)
		{
			Info("- Build Compute Shader: [%s] OK.\n", filename.c_str());
		}
		else
		{
			Info("- Build Compute Shader: [%s] Failed.\n", filename.c_str());
			continue;
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
bool EffectManager::BuildComputeShader(LPCWSTR filename, LPCSTR entrypoint, ID3D11ComputeShader** computeShader)
{
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined( _DEBUG )
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob = NULL;
    ID3DBlob* pBlob = NULL;
    LPCSTR pTarget = (mpd3dDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";
	D3D11_COMPILE_CALL_CHECK(D3DCompileFromFile(filename, NULL, NULL, entrypoint, pTarget, dwShaderFlags, NULL, &pBlob, &pErrorBlob));
	D3D11_CALL_CHECK(mpd3dDevice->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, computeShader));
    if (pErrorBlob) pErrorBlob->Release();
    if (pBlob) pBlob->Release();
	return true;
}