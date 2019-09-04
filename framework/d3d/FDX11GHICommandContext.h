//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "GHIResources.h" 
#include "GHICommandContext.h" 
#include "FDX11GHIResources.h" 
#include "Exceptions.h" 
#include "DX11.h" 

namespace GHI
{
	
	inline D3D11_SAMPLER_DESC  DX11SamplerCast(const GHISamplerDesc  &desc)
	{
		D3D11_SAMPLER_DESC dx11desc;
		dx11desc.AddressU = (D3D11_TEXTURE_ADDRESS_MODE)desc.AddressU;
		dx11desc.AddressV = (D3D11_TEXTURE_ADDRESS_MODE)desc.AddressV;
		dx11desc.AddressW = (D3D11_TEXTURE_ADDRESS_MODE)desc.AddressW;
		dx11desc.BorderColor[0] = desc.BorderColor[0];
		dx11desc.BorderColor[1] = desc.BorderColor[1];
		dx11desc.BorderColor[2] = desc.BorderColor[2];
		dx11desc.BorderColor[3] = desc.BorderColor[3];
		dx11desc.ComparisonFunc = (D3D11_COMPARISON_FUNC)desc.ComparisonFunc;
		dx11desc.Filter = (D3D11_FILTER)desc.Filter;
		dx11desc.MaxAnisotropy = desc.MaxAnisotropy;
		dx11desc.MaxLOD = desc.MaxLOD;
		dx11desc.MinLOD = desc.MinLOD;
		dx11desc.MipLODBias = desc.MipLODBias;
		return dx11desc;
	}

	class FDX11IGHIComputeCommandCotext: public IGHIComputeCommandCotext
	{

	public:
		virtual void SetShaderResource(GHITexture *resource, int slot, GHISRVParam view, EShaderStage stage = EShaderStage::CS) override
		{
			FDX11GHITexture *res = ResourceCast(resource);
			if (res)
			{
                res->view->CreateSRV(view);
                if (stage==EShaderStage::CS)
                    DX11::ImmediateContext()->CSSetShaderResources(slot, 1, &res->rawSRV);
                else if (stage==EShaderStage::PS) 
                    DX11::ImmediateContext()->PSSetShaderResources(slot, 1, &res->rawSRV);

			}
            else
            {
                ELOG("GHI Texture cast failed.");
            }
		}

        virtual void SetShaderResource(GHITexture *resource, int slot, GHIUAVParam view,EShaderStage stage = EShaderStage::CS) override
		{
			FDX11GHITexture *res = ResourceCast(resource);
			if (res)
			{
                res->view->CreateUAV(view);
                if (stage==EShaderStage::CS)
                    DX11::ImmediateContext()->CSSetUnorderedAccessViews(slot, 1, &(res->rawUAV), nullptr);
			}
            else
            {
                ELOG("GHI Texture cast failed.");
            }
		}

        virtual void SetConstBuffer(GHIBuffer *resource, int slot, GHIShader *shader = nullptr) override
        {
			FDX11GHIBuffer *res = ResourceCast(resource);
			if (res)
			{
                if (shader && shader->info.shaderstage == EShaderStage::VS)
                {
                    DX11::ImmediateContext()->VSSetConstantBuffers(slot, 1, &res->rawBuffer);
                    DLOG("Set VS Const Buffer OK.");
                }
                else if (shader && shader->info.shaderstage == EShaderStage::PS)
                {
                    DX11::ImmediateContext()->PSSetConstantBuffers(slot, 1, &res->rawBuffer);
                    DLOG("Set PS Const Buffer OK.");
                }
                else
                {
                    DX11::ImmediateContext()->CSSetConstantBuffers(slot, 1, &res->rawBuffer);
                    DLOG("Set PS Const Buffer OK.");
                }
			}
            else
            {
                ELOG("GHI Buffer cast failed.");
            }
        }

        virtual GHIBuffer* CreateConstBuffer(int size, const void* initData) override
        {
            D3D11_BUFFER_DESC descConstBuffer;
            ZeroMemory(&descConstBuffer, sizeof(descConstBuffer));
            descConstBuffer.ByteWidth = ((size + 15) / 16) * 16; // Caution! size needs to be multiple of 16.
            descConstBuffer.Usage = D3D11_USAGE_DYNAMIC;
            descConstBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            descConstBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            D3D11_SUBRESOURCE_DATA InitData;
            InitData.pSysMem = initData;
            InitData.SysMemPitch = 0;
            InitData.SysMemSlicePitch = 0;

	        ID3D11Buffer *constBuffer = nullptr;
            if (initData == nullptr)
                DXCall(DX11::Device()->CreateBuffer(&descConstBuffer, nullptr, &constBuffer));
            else
                DXCall(DX11::Device()->CreateBuffer(&descConstBuffer, &InitData, &constBuffer));
            
            DLOG("Create Uniform Buffer OK.");
            return new FDX11GHIBuffer(constBuffer);
        }

        virtual void UpdateBuffer(GHIBuffer*buffer, void* data, int size) override
        {
            FDX11GHIBuffer *res = ResourceCast(buffer);
            if (res)
            {
                res->Update(data, size);
            }
            else
            {
                //! cast failed
                ELOG("GHI GHI Buffer cast failed.");
            }
        }

		virtual GHISampler* CreateSampler(const GHISamplerDesc  &desc) override
		{
			D3D11_SAMPLER_DESC descDX11 = DX11SamplerCast(desc);
			ID3D11SamplerState *sampler = nullptr;
			DXCall(DX11::Device()->CreateSamplerState(&descDX11, &sampler));
            DLOG("Create Sampler OK.");
			return new FDX11GHISampler(sampler);
		}
		virtual void SetSampler(GHISampler *resource, int slot, EShaderStage stage) override
		{
            FDX11GHISampler *res = ResourceCast(resource);
            if (res)
            {
				if (EShaderStage::CS == stage)
					DX11::ImmediateContext()->CSSetSamplers(slot, 1, &(res->rawSampler) );
				else if (EShaderStage::PS == stage)
					DX11::ImmediateContext()->PSSetSamplers(slot, 1, &(res->rawSampler) );
            }
            else
            {
                ELOG("GHI GHI Sampler cast failed.");
            }

		}

        virtual GHITexture* CreateTexture(std::string filename) override;
        virtual GHITexture* CreateTextureByAnother(GHITexture * tex) override;
		virtual void CopyTexture(GHITexture *dst, GHITexture *src) override;

        virtual void Dispatch(int nX, int nY, int nZ) override
        {
            DX11::ImmediateContext()->Dispatch( nX, nY, nZ);
        }

		virtual void setPrimitiveTopology(PrimitiveTopology topology) override;
        virtual void SetViewport(GHIViewport viewport) override;
        virtual void Draw(int count, int offset) override;
		virtual void DrawIndexed(int count, int startIndexLocation, int baseIndexLocation) override;
        virtual void DrawIndexedInstanced(unsigned int IndexCountPerInstance, unsigned int InstanceCount,
            unsigned int StartIndexLocation, int BaseVertexLocation, unsigned int StartInstanceLocation
        ) override;

        virtual GHIVertexShader*  CreateVertexShader(std::string file, std::string entrypoint) override;
        virtual GHIPixelShader*   CreatePixelShader(std::string file, std::string entrypoint) override;
        virtual GHIShader* CreateComputeShader(std::string file) override;
        virtual GHIShader* CreateShader(std::string file) override;
        virtual void SetShader(GHIShader* shader) override;

        virtual GHIBuffer*  CreateVertexBuffer(int size, const void* initData) override;
        virtual GHIBuffer*  CreateIndexBuffer(int size, const void* initData) override;
		virtual void SetIndexBuffer(GHIBuffer *buffer, GHIIndexType type, int offset) override;
		virtual void SetVertexBuffers(int startSlot, int numSlots, GHIBuffer *buffer[], int strides[], int offsets[]) override;

        virtual GHIVertexLayout* CreateVertextLayout(const std::vector<GHIInputElementInfo>& vertexFormats, GHIVertexShader *vs) override;

        virtual void SetVertexLayout(GHIVertexLayout* shader) override;

        virtual void SetRenderTarget(int num, GHITexture **colorBuffers, GHITexture *depthBuffer) override;
        virtual void ClearRenderTarget(int num, GHITexture **colorBuffers, float*clearValue = nullptr) override;
        virtual void ClearDepthStencil(GHITexture *depthBuffers, float depth, int stencil, int flag) override;
	};
}