#include "EffectManager.h"
#include "Logger.h"

#include <io.h>
#include <iostream>

void EffectManager::CheckEffect()
{
    for (auto it = mEffects.begin(); it != mEffects.end(); ++it)
        Logger::getLogger() << it->first << " <===> " << it->second << '\n';
}

static wchar_t* CharPtrToLPCWSTR(const char* charArray)
{
    wchar_t* wString = new wchar_t[4096];
    MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
    return wString;
}

static void getFiles(std::string path, std::vector<std::string>& files)
{
    long   hFile = 0;
    struct _finddata_t fileinfo;
    std::string p;
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
    {
        do
        {
            if ((fileinfo.attrib &  _A_SUBDIR)) // subdir
            {
            }
            else
            {
                files.push_back(p.assign(path).append("\\").append(fileinfo.name));
            }
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }
}

EffectPtr EffectManager::NextEffect()
{
    auto ret =  mCurrentEffect == mEffects.end() ? --mCurrentEffect : mCurrentEffect++;
    Logger::getLogger() << "- Next Effect: " << ret->first << "\n";
    return ret->second;

}
EffectPtr EffectManager::PrevEffect()
{
    auto ret =  mCurrentEffect == mEffects.begin() ? mCurrentEffect : --mCurrentEffect;
    Logger::getLogger() << "- Prev Effect: " << ret->first << "\n";
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
    int n = LoadEffectFileList("C:\\Users\\ding\\Documents\\GitHub\\DX11ComputeShaderForImageFilters\\effect");
    ClearEffects();
    for (int i = 0; i < n; ++i)
    {
        std::string filename = mFileList[i];
        LPCWSTR wfilename = CharPtrToLPCWSTR(filename.c_str());
        EffectPtr ptrEffect = NULL;
        Logger::getLogger() << "- Build Compute Shader:" << filename << '\n';
        LoadComputeShader(wfilename, "CSMain", &ptrEffect);
        delete[] wfilename;

        auto ret = mEffects.insert( std::pair<std::string, EffectPtr>(filename, ptrEffect) );
        if (ret.second == false)
        {
            Logger::getLogger() << "- Effect exist:" << filename << '\n';
        }
    }
    mCurrentEffect = mEffects.begin();
}


/*
*	Load a compute shader from  file and use CSMain as entry point
*/
void EffectManager::LoadComputeShader(LPCWSTR filename, LPCSTR entrypoint, ID3D11ComputeShader** computeShader)
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
        Logger::getLogger() << "- Compile Compute Shader Failed." << filename << '\n';
    }
    else
    {
        hr = mpd3dDevice->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, computeShader);
        if (pErrorBlob) pErrorBlob->Release();
        if (pBlob) pBlob->Release();
    }
}