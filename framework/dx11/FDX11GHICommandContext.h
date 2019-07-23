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

namespace SimpleFramework
{
	
	class FDX11IGHIComputeCommandCotext: public IGHIComputeCommandCotext
	{

	public:
		virtual void SetShaderResource(GHITexture *resource, int slot, GHISRVParam view) override
		{
			FDX11GHITexture *res = ResourceCast(resource);
			if (res)
			{
                res->view->CreateSRV(view);
                DX11::ImmediateContext()->CSSetShaderResources(slot, 1, &res->rawSRV);
			}
            else
            {
                //! cast failed
            }

		}

        virtual void SetShaderResource(GHITexture *resource, int slot, GHIUAVParam view) override
		{
			FDX11GHITexture *res = ResourceCast(resource);
			if (res)
			{
                res->view->CreateUAV(view);
                DX11::ImmediateContext()->CSSetUnorderedAccessViews(slot, 1, &(res->rawUAV), nullptr);
			}
            else
            {
                //! cast failed
            }

		}

        virtual void SetConstBuffer(GHIBuffer *resource, int slot) override
        {
			FDX11GHIBuffer *res = ResourceCast(resource);
			if (res)
			{
                DX11::ImmediateContext()->CSSetConstantBuffers(slot, 1, &res->rawBuffer);
			}
            else
            {
                //! cast failed
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
            DXCall(DX11::Device()->CreateBuffer(&descConstBuffer, &InitData, &constBuffer));
            
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
            }
        }

        virtual void Dispatch(int nX, int nY, int nZ) override
        {
            DX11::ImmediateContext()->Dispatch( nX, nY, nZ);
        }
	};
}