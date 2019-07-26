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
}