//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "FDX11GHIResources.h" 
#include "DX11.h" 

namespace SimpleFramework
{

	void FDX11GHITexture::LoadFromFile(std::string filename)
	{
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