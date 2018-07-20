#include "EffectManager.h"
#include <io.h>
#include <iostream>

EffectPtr* EffectManager::GetEffect()
{
    for (auto it = mEffects.begin(); it != mEffects.end(); ++it)
        std::cout << it->first << " <=> " << it->second << '\n';
    return NULL;
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

int EffectManager::LoadEffectFileList(std::string dir)
{
    getFiles(dir, mFileList);
    return mFileList.size();
}

void EffectManager::BuildEffects()
{
    int n = LoadEffectFileList("C:\\Users\\ding\\Documents\\GitHub\\DX11ComputeShaderForImageFilters\\effect");
    for (int i = 0; i < n; ++i)
    {
        std::string filename = mFileList[i];
        LPCWSTR wfilename = CharPtrToLPCWSTR(filename.c_str());
        EffectPtr ptrEffect;
        LoadComputeShader(wfilename, "CSMain", &ptrEffect);
        delete[] wfilename;

        auto ret = mEffects.insert( std::pair<std::string, EffectPtr>(filename, ptrEffect) );
        if (ret.second == false)
        {
            printf("- already exist.\n");
        }
    }
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
        printf("- Compile Compute Shader Failed.\n");
    }
    else
    {
        hr = mpd3dDevice->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, computeShader);
        if (pErrorBlob) pErrorBlob->Release();
        if (pBlob) pBlob->Release();
    }
}