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
#include "Utility.h" 
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"

using namespace DirectX;

namespace GHI
{

	void FDX11GHITexture::LoadFromFile(std::string filename)
	{
		std::wstring wName = StrToWstr(filename.c_str());

        if (ExtractExtension(filename) == "dds")
        {
            DXCall(CreateDDSTextureFromFile(DX11::Device(), wName.c_str(), (ID3D11Resource **)&rawTexture, &rawSRV));
        }
        else
        {
		    DXCall(CreateWICTextureFromFile(DX11::Device(), wName.c_str(), (ID3D11Resource **)&rawTexture, &rawSRV));
        }

		D3D11_TEXTURE2D_DESC desc;
		rawTexture->GetDesc(&desc);
		//AssertMsg_((desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM), "Texutre format != DXGI_FORMAT_R8G8B8A8_UNORM");
        if (desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM && desc.Format != DXGI_FORMAT_B8G8R8X8_UNORM)
        {
            width = height = textureSizeInBytes = 0;
            aspect = 0.;
            ELOG("Texutre format != DXGI_FORMAT_R8G8B8A8_UNORM.");
			return;
        }
		width  = desc.Width;
		height = desc.Height;
		aspect = float(width) / float(height);
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
		if (res->rawSRV)
        {
        }
		else
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
			ZeroMemory(&viewDesc, sizeof(viewDesc));
			viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipLevels = 1;
			viewDesc.Texture2D.MostDetailedMip = 0;
			DXCall(DX11::Device()->CreateShaderResourceView(res->rawTexture, &viewDesc, &(res->rawSRV)));
        }
	}
	void FDX11GHIResourceView::CreateUAV(const GHIUAVParam &param)
	{
		FDX11GHITexture *res = ResourceCast(resource);
		if (res->rawUAV)
        {
        }
		else
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC descView;
			ZeroMemory(&descView, sizeof(descView));
			descView.Texture2D = { 0 };
			descView.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			descView.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

			DXCall(DX11::Device()->CreateUnorderedAccessView(res->rawTexture, &descView, &(res->rawUAV)));
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