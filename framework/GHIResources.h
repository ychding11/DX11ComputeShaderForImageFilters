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

	enum EUAVDemension
	{
        UAVDimension_TEXTURE2D,

	};

    struct GHIUAVParam
    {
        EPixelFormat Format;
        EUAVDemension ViewDimension;
		uint32_t MostDetailedMip;
		uint32_t MipLevels;
    };
    struct GHISRVParam
    {
        EPixelFormat Format;
        EUAVDemension ViewDimension;
		uint32_t MostDetailedMip;
		uint32_t MipLevels;

    };
    struct GHIRTVParam
    {
        EPixelFormat Format;
        EUAVDemension ViewDimension;
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