//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include <string>

namespace SimpleFramework
{
	enum EPixelFormat
	{
		PixelFormat_R8G8B8A8_UNORM,

	};

	enum EViewDemension
	{
        EViewDimension_TEXTURE2D,

	};

    enum EResourceUsage
    {
        Default	= 0,
        Immutable = 1,
        Dynamic	= 2,
        Staging	= 3
    };

    class TextureDesc2D
    {
	public:
		uint32_t Width = 0;
		uint32_t Height = 0;
		uint32_t MipLevels = 1;
		uint32_t ArraySize = 1;
		EPixelFormat Format = PixelFormat_R8G8B8A8_UNORM;
		uint32_t SampleCountPixel = 1;
		uint32_t ImageQualityLevel = 0;
		EResourceUsage Usage = EResourceUsage::Default;
		uint32_t BindFlags = 0;
		uint32_t CPUAccessFlags = 0;
		uint32_t MiscFlags = 0;
    };

    struct GHIUAVParam
    {
        EPixelFormat Format;
        EViewDemension ViewDimension;
		uint32_t MostDetailedMip;
		uint32_t MipLevels;
    };
    class GHISRVParam
    {
	public:
        EPixelFormat Format = PixelFormat_R8G8B8A8_UNORM;
        EViewDemension ViewDimension = EViewDimension_TEXTURE2D;
		uint32_t MostDetailedMip = 0;
		uint32_t MipLevels = 1;

    };
    struct GHIRTVParam
    {
        EPixelFormat Format;
        EViewDemension ViewDimension;
		uint32_t MostDetailedMip;
		uint32_t MipLevels;
    };


    class GHIResourceView;
	class GHIResource
	{
	};

    class GHIRenderTargetView
    {

    };

    class GHIShaderResourceView
    {

    };

    class GHIUnorderedAccessView
    {

    };


	class IGHIResourceView
	{
	public:
        virtual void CreateRTV(const GHIRTVParam &) = 0;
        virtual void CreateSRV(const GHISRVParam &) = 0;
        virtual void CreateUAV(const GHIUAVParam &) = 0;
	};

	class GHITexture :public GHIResource
	{
	public:
		IGHIResourceView *view = nullptr;
		virtual void LoadFromFile(std::string filename) = 0;
	};

	class GHIBuffer :public GHIResource
	{
	public:
		virtual void Update(void* data, int size) = 0;
	};


}