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

	enum TextureAddressMode
	{
		WRAP = 1,
		MIRROR = 2,
		CLAMP = 3,
		BORDER = 4,
		MIRROR_ONCE = 5
	};
	enum TextureFilter
	{
		FILTER_MIN_MAG_MIP_POINT = 0,
		FILTER_MIN_MAG_POINT_MIP_LINEAR = 0x1,
		FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x4,
		FILTER_MIN_POINT_MAG_MIP_LINEAR = 0x5,
		FILTER_MIN_LINEAR_MAG_MIP_POINT = 0x10,
		FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x11,
		FILTER_MIN_MAG_LINEAR_MIP_POINT = 0x14,
		FILTER_MIN_MAG_MIP_LINEAR = 0x15,
		FILTER_ANISOTROPIC = 0x55,
	};

	enum ComparisonFunc
	{
		COMPARISON_NEVER = 1,
		COMPARISON_LESS = 2,
		COMPARISON_EQUAL = 3,
		COMPARISON_LESS_EQUAL = 4,
		COMPARISON_GREATER = 5,
		COMPARISON_NOT_EQUAL = 6,
		COMPARISON_GREATER_EQUAL = 7,
		COMPARISON_ALWAYS = 8
	};

	struct GHISamplerDesc
	{
		TextureFilter Filter = TextureFilter::FILTER_MIN_MAG_MIP_LINEAR;
		TextureAddressMode AddressU = TextureAddressMode::WRAP;
		TextureAddressMode AddressV = TextureAddressMode::WRAP;
		TextureAddressMode AddressW = TextureAddressMode::WRAP;
		float MipLODBias = 0;
		uint32_t MaxAnisotropy = 0;
		ComparisonFunc ComparisonFunc = ComparisonFunc::COMPARISON_NEVER;
		float BorderColor[4] = {0, 0, 0, 0};
		float MinLOD = 0;
		float MaxLOD = 1e20;
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

	class GHISampler :public GHIResource
	{

	};

}