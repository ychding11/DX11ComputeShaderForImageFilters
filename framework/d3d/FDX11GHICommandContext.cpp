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

namespace GHI
{
	void FDX11IGHIComputeCommandCotext::setPrimitiveTopology(PrimitiveTopology topology)
	{
		D3D_PRIMITIVE_TOPOLOGY top = (D3D_PRIMITIVE_TOPOLOGY)topology;
		DX11::ImmediateContext()->IASetPrimitiveTopology(top);
        DLOG("Set Primitive Topology.");
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
            ELOG("Copy Texture failed.");
		}
	}

	GHITexture* FDX11IGHIComputeCommandCotext::CreateTexture(std::string filename)
	{
		GHITexture *tex = new FDX11GHITexture(filename);
        DLOG("Load Texture from file.");
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
            DLOG("GHI Texutre cast fail.");
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
        DLOG("Set Viewport");
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
		if (!SUCCEEDED(D3DCompileFromFile(filename, NULL, NULL, entrypoint, pTarget, dwShaderFlags, NULL, pBlob, pErrorBlob)))
		{
			DLOG("Shader Compile Error: %s", (char*)((*pErrorBlob)->GetBufferPointer()) );
		}
	    DXCall(DX11::Device()->CreateComputeShader((*pBlob)->GetBufferPointer(), (*pBlob)->GetBufferSize(), NULL, computeShader));
	    return true;
    }

    static bool BuildPixelShader(LPCWSTR filename, LPCSTR entrypoint, ID3D11PixelShader** shader, ID3DBlob** pBlob, ID3DBlob** pErrorBlob )
    {
        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

        #if defined( DEBUG ) || defined( _DEBUG )
        dwShaderFlags |= D3DCOMPILE_DEBUG;
        #endif

        if (FAILED(D3DCompileFromFile(filename, NULL, NULL, entrypoint, "ps_4_0", dwShaderFlags, 0, pBlob, pErrorBlob)))
        {
            ELOG("Compile Pixel shader fail. [ %s ]", WstrToStr(filename).c_str());
            if (*pErrorBlob)
            {
                ELOG("Compile info:%s",(char*)( (*pErrorBlob)->GetBufferPointer()));
            }
            return false;
        }
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
        if (FAILED(D3DCompileFromFile(filename, NULL, NULL, entrypoint, "vs_4_0", dwShaderFlags, 0, pBlob, pErrorBlob)))
        {
            ELOG("Compile Vertext shader fail. [ %s ]", WstrToStr(filename).c_str());
            if (*pErrorBlob)
            {
                ELOG("Compile info:%s",(char*)( (*pErrorBlob)->GetBufferPointer()));
            }
            return false;
        }
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
            ID3D11ShaderReflectionConstantBuffer* cb = reflection->GetConstantBufferByIndex(i);
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
                        ID3D11ShaderReflectionVariable* variable = cb->GetVariableByIndex(j);
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
        if (!BuildComputeShader(wfile.c_str(), entrypoint.c_str(), &csPtr, &pBlob, &pErrorBlob)) return nullptr;
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

    GHIVertexLayout* FDX11IGHIComputeCommandCotext::CreateVertextLayout(const std::vector<GHIInputElementInfo>& vertexFormats, GHIVertexShader *vs)
    {
        ID3D11InputLayout *layout = nullptr;

        if (vs && vs->info.shaderstage == EShaderStage::VS)
        {
            FDX11GHIVertexShader *shader = dynamic_cast<FDX11GHIVertexShader *>(vs);
            if (!shader)
            {
                ELOG("GHI Shader cast failed.");
                return nullptr;
            }
            const D3D11_INPUT_ELEMENT_DESC *elemDsc = reinterpret_cast<const D3D11_INPUT_ELEMENT_DESC*>(vertexFormats.data());
            DXCall(DX11::Device()->CreateInputLayout(elemDsc, vertexFormats.size(), vs->info.bytecode.data(), vs->info.bytecode.size() , &layout));
            return new FDX11GHIVertexLayout(layout);
        }
        else
        {
            ELOG("shader type is NOT Vertext shader.");
            return nullptr;
        }
    }

    void FDX11IGHIComputeCommandCotext::SetShader(GHIShader* shader)
    {
        if (shader && shader->info.shaderstage == EShaderStage::CS)
        {
            FDX11GHIComputeShader *cs = dynamic_cast<FDX11GHIComputeShader *>(shader);
            if (!cs)
            {
                ELOG("GHI Shader cast failed.");
                return;
            }
            DX11::ImmediateContext()->CSSetShader(cs->rawPtr, nullptr, 0);
            DLOG("Set Compute Shader OK.");
        }
        else if (shader && shader->info.shaderstage == EShaderStage::VS)
        {
            FDX11GHIVertexShader *vs = dynamic_cast<FDX11GHIVertexShader *>(shader);
            if (!vs)
            {
                ELOG("GHI Shader cast failed.");
                return;
            }
            DX11::ImmediateContext()->VSSetShader(vs->rawPtr, nullptr, 0);
            DLOG("Set Vertex Shader OK.");
        }
        else if (shader && shader->info.shaderstage == EShaderStage::PS)
        {
            FDX11GHIPixelShader *ps = dynamic_cast<FDX11GHIPixelShader *>(shader);
            if (!ps)
            {
                ELOG("GHI Shader cast failed.");
                return;
            }

            DX11::ImmediateContext()->PSSetShader(ps->rawPtr, nullptr, 0);
            DLOG("Set Pixel Shader OK.");
        }

    }

	GHIBuffer*  FDX11IGHIComputeCommandCotext::CreateVertexBuffer(int size, const void* initData)
	{
        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth = size;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA data;
        data.pSysMem = initData;
        data.SysMemPitch = 0;
        data.SysMemSlicePitch = 0;

        ID3D11Buffer *vertexBuffer = nullptr;
        DXCall(DX11::Device()->CreateBuffer(&bufferDesc, &data, &vertexBuffer));

        return new FDX11GHIBuffer(vertexBuffer);
	}
	GHIBuffer*  FDX11IGHIComputeCommandCotext::CreateIndexBuffer(int size, const void* initData)
	{
        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth = size;
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA data;
        data.pSysMem = initData;
        data.SysMemPitch = 0;
        data.SysMemSlicePitch = 0;

        ID3D11Buffer *indexBuffer = nullptr;
        DXCall(DX11::Device()->CreateBuffer(&bufferDesc, &data, &indexBuffer));

        return new FDX11GHIBuffer(indexBuffer);
	}

	void FDX11IGHIComputeCommandCotext::SetIndexBuffer(GHIBuffer *buffer, GHIIndexType type, int offset)
	{
        FDX11GHIBuffer *res = ResourceCast(buffer);
        if (res)
        {
            DXGI_FORMAT indexFormat = type == GHIIndexType::Index32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
            DX11::ImmediateContext()->IASetIndexBuffer(res->rawBuffer, indexFormat, offset);
        }
        else
        {
           ELOG("GHI Index Buffer cast failed.");
           return;
        }
        DLOG("Set Index Buffer OK.");
	}

	void FDX11IGHIComputeCommandCotext::SetVertexBuffers(int startSlot, int numSlots, GHIBuffer *buffer[], int strides[], int offsets[])
	{
        // check parameter here

        ID3D11Buffer * devicebuffer[8] = { 0 };
        for (int i = 0; i < numSlots; ++i)
        {
            FDX11GHIBuffer *res = ResourceCast(buffer[i]);
            if (res)
            {
                devicebuffer[i] = res->rawBuffer;
            }
            else
            {
                ELOG("GHI Vertext Buffer cast failed.");
                return;
            }

        }
        DX11::ImmediateContext()->IASetVertexBuffers(startSlot, numSlots, devicebuffer, (unsigned int*)strides, (unsigned int*)offsets);
        DLOG("Set Vetex Buffer OK. slot[%d,%d]", startSlot, numSlots);
	}

	void FDX11IGHIComputeCommandCotext::DrawIndexed(int count, int startIndexLocation, int baseIndexLocation)
	{
        DX11::ImmediateContext()->DrawIndexed(count, startIndexLocation, baseIndexLocation);
	}
    
    void FDX11IGHIComputeCommandCotext::DrawIndexedInstanced(unsigned int IndexCountPerInstance, unsigned int InstanceCount,
                                                             unsigned int StartIndexLocation, int BaseVertexLocation, unsigned int StartInstanceLocation
    )
    {
        DX11::ImmediateContext()->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
    }

    void FDX11IGHIComputeCommandCotext::SetVertexLayout(GHIVertexLayout* layout)
    {
        FDX11GHIVertexLayout *res = ResourceCast(layout);
        if (res)
        {
            ID3D11InputLayout *raw = res->rawLayout;
            DX11::ImmediateContext()->IASetInputLayout(raw);
        }
        else
        {
            ELOG("GHI Vertex Layout cast failed.");
            return;
        }
        DLOG("Set Vetex Layout OK.");
    }

    void FDX11IGHIComputeCommandCotext::SetRenderTarget(int num, GHITexture **colorBuffers, GHITexture *depthBuffer)
    {
        AssertMsg_(num > 0, "render target number error.");
        ID3D11DepthStencilView *depth = nullptr;
        std::vector<ID3D11RenderTargetView*> colors;
        colors.resize(num);
        for (int i = 0; i < num; ++i)
        {
            FDX11GHITexture *res = ResourceCast(colorBuffers[i]);
            if (res == nullptr)
            {
                ELOG("GHI Texture cast fail.");
                return;
            }
            colors[i] = res->rawRTV;
        }

        FDX11GHITexture *res = ResourceCast(depthBuffer);
        if (res == nullptr && depthBuffer != nullptr)
        {
            ELOG("GHI Texture cast fail.");
            return;
        }

        DX11::ImmediateContext()->OMSetRenderTargets(num, colors.data(), nullptr); // hardcode, need refine
    }
    void FDX11IGHIComputeCommandCotext::ClearRenderTarget(int num, GHITexture **colorBuffers, float*clearValue)
    {
        std::vector<ID3D11RenderTargetView*> colors;
        colors.resize(num);
        float defaultColor[4] = { 0 };
        for (int i = 0; i < num; ++i)
        {
            FDX11GHITexture *res = ResourceCast(colorBuffers[i]);
            if (res == nullptr)
            {
                ELOG("GHI Texture cast fail.");
                return;
            }
            colors[i] = res->rawRTV;
            if (clearValue == nullptr)
            {
                clearValue = defaultColor;
            }
            DX11::ImmediateContext()->ClearRenderTargetView(res->rawRTV, clearValue);
        }

    }

    void FDX11IGHIComputeCommandCotext::ClearDepthStencil(GHITexture *depthBuffers, float depth, int stencil, int flag)
    {

    }
}