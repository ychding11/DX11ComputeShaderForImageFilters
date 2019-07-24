//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "FDX11GHIResources.h" 
#include "DX11.h" 
#include "Exceptions.h" 
#include "WICTextureLoader.h"

using namespace DirectX;

namespace SimpleFramework
{

	void FDX11GHITexture::LoadFromFile(std::wstring filename)
	{
		DXCall(CreateWICTextureFromFile(DX11::Device(), filename.c_str(), (ID3D11Resource **)&rawTexture, &rawSRV));
		D3D11_TEXTURE2D_DESC desc;
		rawTexture->GetDesc(&desc);
		if (desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM)
		{
			//Info("Load [%s]: Texture format NOT qualified: DXGI_FORMAT_R8G8B8A8_UNORM @%s:%d\n", m_imageName.c_str(), __FILE__, __LINE__);
		}
		aspect = float(desc.Width) / float(desc.Height);
		textureSizeInBytes = desc.Width * desc.Height * 4;
	}

	void FDX11GHIResourceView::CreateRTV(const GHIRTVParam &param)
	{
		FDX11GHITexture *res = ResourceCast(resource);
		if (res->rawRTV)
        {
        }
		else
		{

        }
	}

	void FDX11GHIResourceView::CreateSRV(const GHISRVParam &param)
	{
		FDX11GHITexture *res = ResourceCast(resource);
		if (res->rawRTV)
        {
        }
		else
		{

        }
	}
	void FDX11GHIResourceView::CreateUAV(const GHIUAVParam &param)
	{
		FDX11GHITexture *res = ResourceCast(resource);
		if (res->rawRTV)
        {
        }
		else
		{

        }
	}

    void FDX11GHIBuffer::Update(void* data, int size)
    {
        D3D11_MAPPED_SUBRESOURCE MappedResource;
        DX11::ImmediateContext()->Map(rawBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
        auto pData = reinterpret_cast<void*>(MappedResource.pData);
        memcpy(pData,data, size);
        DX11::ImmediateContext()->Unmap(rawBuffer, 0);
    }
}