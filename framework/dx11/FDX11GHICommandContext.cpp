//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "GHICommandContext.h" 
#include "FDX11GHICommandContext.h" 
#include "FDX11GHIResources.h" 

namespace SimpleFramework
{
	void FDX11IGHIComputeCommandCotext::setPrimitiveTopology(PrimitiveTopology topology)
	{
		D3D_PRIMITIVE_TOPOLOGY top = (D3D_PRIMITIVE_TOPOLOGY)topology;
		DX11::ImmediateContext()->IASetPrimitiveTopology(top);
	}

	void FDX11IGHIComputeCommandCotext::CopyTexture(GHITexture *dst, GHITexture *src)
	{
        FDX11GHITexture *dtex = ResourceCast(dst);
        FDX11GHITexture *stex = ResourceCast(src);
		if (dtex && stex)
		{
			DX11::ImmediateContext()->CopyResource(dtex->rawTexture, stex->rawTexture);
		}
		else
		{

		}
	}

	GHITexture* FDX11IGHIComputeCommandCotext::CreateTexture(std::string filename)
	{
		GHITexture *tex = new FDX11GHITexture(filename);
		return tex;
	}

	GHITexture* FDX11IGHIComputeCommandCotext::CreateTextureByAnother(GHITexture * tex)
	{
            FDX11GHITexture *res = ResourceCast(tex);
			if (res)
			{
				ID3D11Texture2D *rawTexture = res->rawTexture;
				D3D11_TEXTURE2D_DESC desc;
				rawTexture->GetDesc(&desc);
				desc.BindFlags |= (D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE);
				ID3D11Texture2D *temp = nullptr;
				DXCall(DX11::Device()->CreateTexture2D(&desc, NULL, &temp));
				return new FDX11GHITexture(temp);
			}
			else
			{

				return nullptr;
			}
	}

    void FDX11IGHIComputeCommandCotext::SetViewport(GHIViewport viewport)
    {
        D3D11_VIEWPORT vp;
        vp.Width = viewport.Width; vp.Height = viewport.Height;
        vp.TopLeftX = viewport.TopLeftX; vp.TopLeftY = viewport.TopLeftY;
        vp.MinDepth = viewport.MinDepth; vp.MaxDepth = viewport.MaxDepth;
        DX11::ImmediateContext()->RSSetViewports(1, &vp);
    }
    void FDX11IGHIComputeCommandCotext::Draw(int count, int offset)
    {
        DX11::ImmediateContext()->Draw(count, offset);
    }


    static bool BuildComputeShader(LPCWSTR filename, LPCSTR entrypoint, ID3D11ComputeShader** computeShader,ID3DBlob** pBlob,ID3DBlob** pErrorBlob )
    {
        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
    #if defined( _DEBUG )
        dwShaderFlags |= D3DCOMPILE_DEBUG;
    #endif

        LPCSTR pTarget = (DX11::Device()->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";
	    DXCall(D3DCompileFromFile(filename, NULL, NULL, entrypoint, pTarget, dwShaderFlags, NULL, pBlob, pErrorBlob));
	    DXCall(DX11::Device()->CreateComputeShader((*pBlob)->GetBufferPointer(), (*pBlob)->GetBufferSize(), NULL, computeShader));
	    return true;
    }

    static bool BuildPixelShader(LPCWSTR filename, LPCSTR entrypoint, ID3D11PixelShader** shader, ID3DBlob** pBlob, ID3DBlob** pErrorBlob )
    {
        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

        #if defined( DEBUG ) || defined( _DEBUG )
        dwShaderFlags |= D3DCOMPILE_DEBUG;
        #endif

        DXCall(D3DCompileFromFile(filename, NULL, NULL, entrypoint, "ps_4_0", dwShaderFlags, 0, pBlob, pErrorBlob));
        DXCall(DX11::Device()->CreatePixelShader((*pBlob)->GetBufferPointer(), (*pBlob)->GetBufferSize(), NULL, shader));

        DLOG("Build pixel Shader['%ls'] OK.\n", filename);

        return true;
    }

    static bool BuildVertexShader(LPCWSTR filename, LPCSTR entrypoint, ID3D11VertexShader** shader, ID3DBlob** pBlob, ID3DBlob** pErrorBlob )
    {
        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined( DEBUG ) || defined( _DEBUG )
        dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
        DXCall(D3DCompileFromFile(filename, NULL, NULL, entrypoint, "vs_4_0", dwShaderFlags, 0, pBlob, pErrorBlob));
        DXCall(DX11::Device()->CreateVertexShader((*pBlob)->GetBufferPointer(), (*pBlob)->GetBufferSize(), NULL, shader));
        DLOG("Build vertex Shader['%ls'] OK.\n", filename);
        
        return true;
    }

    static void EnumRelection(GHIShader *shader)
    {
        ID3D11ShaderReflection* reflection = nullptr;
        D3DReflect(shader->info.bytecode.data(), shader->info.bytecode.size(), IID_ID3D11ShaderReflection, (void**)&reflection);

        D3D11_SHADER_DESC desc;
        reflection->GetDesc(&desc);

        DLOG("Shader file:%s",shader->info.shaderfile.c_str());
        for (unsigned int i = 0; i < desc.ConstantBuffers; ++i)
        {
            unsigned int register_index = 0;
            ID3D11ShaderReflectionConstantBuffer* cb = nullptr;
            cb = reflection->GetConstantBufferByIndex(i);

            D3D11_SHADER_BUFFER_DESC cbDesc;
            cb->GetDesc(&cbDesc);

            for (unsigned int k = 0; k < desc.BoundResources; ++k)
            {
                D3D11_SHADER_INPUT_BIND_DESC ibdesc;
                reflection->GetResourceBindingDesc(k, &ibdesc);

                DLOG("\tparam name:%s",ibdesc.Name);
                DLOG("\tbind point:%d",ibdesc.BindPoint);
                DLOG("\tbind count:%d",ibdesc.BindCount);
                DLOG("\tparam type:%d",ibdesc.Type);
                if (!strcmp(ibdesc.Name, cbDesc.Name))
                {
                    register_index = ibdesc.BindPoint;
                    for (unsigned int j = 0; j < cbDesc.Variables; ++j)
                    {
                        ID3D11ShaderReflectionVariable* variable = NULL;
                        variable = cb->GetVariableByIndex(j);

                        D3D11_SHADER_VARIABLE_DESC vdesc;
                        variable->GetDesc(&vdesc);
                        DLOG("\tvariable name:%s", vdesc.Name);
                        DLOG("\tstart offset:%d", vdesc.StartOffset);
                        DLOG("\tvariable size:%d", vdesc.Size);
#if 0
                        ShaderVariable* shadervariable = new ShaderVariable();
                        shadervariable->name = Engine::String.ConvertToWideStr(vdesc.Name);
                        shadervariable->length = vdesc.Size;
                        shadervariable->offset = vdesc.StartOffset;
                        mSize += vdesc.Size;
                        mVariables.push_back(shadervariable);
#endif
                    }
                }
            }
        }
    }

    GHIVertexShader*  FDX11IGHIComputeCommandCotext::CreateVertexShader(std::string file, std::string entrypoint)
    {
        std::wstring wfile = StrToWstr(file.c_str());
        ID3D11VertexShader *vsPtr = nullptr;
        ID3DBlob* pErrorBlob = nullptr;
        ID3DBlob* pBlob = nullptr;
        BuildVertexShader(wfile.c_str(), entrypoint.c_str(), &vsPtr, &pBlob, &pErrorBlob);
        GHIVertexShader *shader = new FDX11GHIVertexShader(vsPtr);
		shader->info.shaderfile = file;
		shader->info.entrypoint = entrypoint;
        shader->info.shaderstage = EShaderStage::VS;
		shader->info.bytecode += std::string((char*)(pBlob->GetBufferPointer()), pBlob->GetBufferSize());
		return shader;
    }
    GHIPixelShader*   FDX11IGHIComputeCommandCotext::CreatePixelShader(std::string file, std::string entrypoint)
    {
        std::wstring wfile = StrToWstr(file.c_str());
        ID3D11PixelShader *psPtr = nullptr;
        ID3DBlob* pErrorBlob = nullptr;
        ID3DBlob* pBlob = nullptr;
        BuildPixelShader(wfile.c_str(), entrypoint.c_str(), &psPtr, &pBlob, &pErrorBlob);
        GHIPixelShader *shader = new FDX11GHIPixelShader(psPtr);
		shader->info.shaderfile = file;
		shader->info.entrypoint = entrypoint;
        shader->info.shaderstage = EShaderStage::PS;
		shader->info.bytecode += std::string((char*)(pBlob->GetBufferPointer()), pBlob->GetBufferSize());
		return shader;
    }

    GHIShader* FDX11IGHIComputeCommandCotext::CreateComputeShader(std::string file)
    {
        std::wstring wfile = StrToWstr(file.c_str());
		std::string entrypoint = "CSMain";
        ID3D11ComputeShader *csPtr = nullptr;
        ID3DBlob* pErrorBlob = nullptr;
        ID3DBlob* pBlob = nullptr;
        BuildComputeShader(wfile.c_str(), entrypoint.c_str(), &csPtr, &pBlob, &pErrorBlob);
        GHIShader *shader = new FDX11GHIComputeShader(csPtr);
		shader->info.shaderfile = file;
		shader->info.entrypoint = entrypoint;
        shader->info.shaderstage = EShaderStage::CS;
		shader->info.bytecode += std::string((char*)(pBlob->GetBufferPointer()), pBlob->GetBufferSize());
        EnumRelection(shader);
		return shader;
    }

	GHIShader* FDX11IGHIComputeCommandCotext::CreateShader(std::string file)
	{
        std::wstring wfile = StrToWstr(file.c_str());
		std::string entrypoint = "CSMain";
        ID3D11ComputeShader *csPtr = nullptr;
        ID3DBlob* pErrorBlob = nullptr;
        ID3DBlob* pBlob = nullptr;
        BuildComputeShader(wfile.c_str(), entrypoint.c_str(), &csPtr, &pBlob, &pErrorBlob);
        GHIShader *shader = new FDX11GHIComputeShader(csPtr);
		shader->info.shaderfile = file;
		shader->info.entrypoint = entrypoint;
        shader->info.shaderstage = EShaderStage::CS;
		shader->info.bytecode += std::string((char*)(pBlob->GetBufferPointer()), pBlob->GetBufferSize());
        EnumRelection(shader);
		return shader;
	}

    void FDX11IGHIComputeCommandCotext::SetShader(GHIShader* shader)
    {
        if (shader && shader->info.shaderstage == EShaderStage::CS)
        {
            FDX11GHIComputeShader *cs = dynamic_cast<FDX11GHIComputeShader *>(shader);
            if (!cs)
            {
                return;
            }

            DX11::ImmediateContext()->CSSetShader(cs->rawPtr, nullptr, 0);
        }
        else if (shader && shader->info.shaderstage == EShaderStage::VS)
        {
            FDX11GHIVertexShader *vs = dynamic_cast<FDX11GHIVertexShader *>(shader);
            if (!vs)
            {
                return;
            }

            DX11::ImmediateContext()->VSSetShader(vs->rawPtr, nullptr, 0);
        }
        else if (shader && shader->info.shaderstage == EShaderStage::PS)
        {
            FDX11GHIPixelShader *ps = dynamic_cast<FDX11GHIPixelShader *>(shader);
            if (!ps)
            {
                return;
            }

            DX11::ImmediateContext()->PSSetShader(ps->rawPtr, nullptr, 0);
        }

    }
}