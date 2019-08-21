//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "GHIResources.h" 

namespace GHI
{
    typedef float               FLOAT;

    enum EResourceView
    {
        RTV,
        UAV,
        SRV,
        NUM
    };

    struct GHIViewport
    {
        FLOAT TopLeftX = 0;
        FLOAT TopLeftY = 0;
        FLOAT Width = 0;
        FLOAT Height = 0;
        FLOAT MinDepth = 0.;
        FLOAT MaxDepth = 1.;
    };

    enum GHIIndexType
    {
        Index16Bit,
        Index32Bit
    };


    enum GHIDataFormat
    {
        DATA_FORMAT_UNKNOWN = 0,
        DATA_FORMAT_R32G32B32A32_TYPELESS = 1,
        DATA_FORMAT_R32G32B32A32_FLOAT = 2,
        DATA_FORMAT_R32G32B32A32_UINT = 3,
        DATA_FORMAT_R32G32B32A32_SINT = 4,
        DATA_FORMAT_R32G32B32_TYPELESS = 5,
        DATA_FORMAT_R32G32B32_FLOAT = 6,
        DATA_FORMAT_R32G32B32_UINT = 7,
        DATA_FORMAT_R32G32B32_SINT = 8,
        DATA_FORMAT_R16G16B16A16_TYPELESS = 9,
        DATA_FORMAT_R16G16B16A16_FLOAT = 10,
        DATA_FORMAT_R16G16B16A16_UNORM = 11,
        DATA_FORMAT_R16G16B16A16_UINT = 12,
        DATA_FORMAT_R16G16B16A16_SNORM = 13,
        DATA_FORMAT_R16G16B16A16_SINT = 14,
        DATA_FORMAT_R32G32_TYPELESS = 15,
        DATA_FORMAT_R32G32_FLOAT = 16,
        DATA_FORMAT_R32G32_UINT = 17,
        DATA_FORMAT_R32G32_SINT = 18,
        DATA_FORMAT_R32G8X24_TYPELESS = 19,
        DATA_FORMAT_D32_FLOAT_S8X24_UINT = 20,
        DATA_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
        DATA_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
        DATA_FORMAT_R10G10B10A2_TYPELESS = 23,
        DATA_FORMAT_R10G10B10A2_UNORM = 24,
        DATA_FORMAT_R10G10B10A2_UINT = 25,
        DATA_FORMAT_R11G11B10_FLOAT = 26,
        DATA_FORMAT_R8G8B8A8_TYPELESS = 27,
        DATA_FORMAT_R8G8B8A8_UNORM = 28,
        DATA_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
        DATA_FORMAT_R8G8B8A8_UINT = 30,
        DATA_FORMAT_R8G8B8A8_SNORM = 31,
        DATA_FORMAT_R8G8B8A8_SINT = 32,
        DATA_FORMAT_R16G16_TYPELESS = 33,
        DATA_FORMAT_R16G16_FLOAT = 34,
        DATA_FORMAT_R16G16_UNORM = 35,
        DATA_FORMAT_R16G16_UINT = 36,
        DATA_FORMAT_R16G16_SNORM = 37,
        DATA_FORMAT_R16G16_SINT = 38,
        DATA_FORMAT_R32_TYPELESS = 39,
        DATA_FORMAT_D32_FLOAT = 40,
        DATA_FORMAT_R32_FLOAT = 41,
        DATA_FORMAT_R32_UINT = 42,
        DATA_FORMAT_R32_SINT = 43,
        DATA_FORMAT_R24G8_TYPELESS = 44,
        DATA_FORMAT_D24_UNORM_S8_UINT = 45,
        DATA_FORMAT_R24_UNORM_X8_TYPELESS = 46,
        DATA_FORMAT_X24_TYPELESS_G8_UINT = 47,
        DATA_FORMAT_R8G8_TYPELESS = 48,
        DATA_FORMAT_R8G8_UNORM = 49,
        DATA_FORMAT_R8G8_UINT = 50,
        DATA_FORMAT_R8G8_SNORM = 51,
        DATA_FORMAT_R8G8_SINT = 52,
        DATA_FORMAT_R16_TYPELESS = 53,
        DATA_FORMAT_R16_FLOAT = 54,
        DATA_FORMAT_D16_UNORM = 55,
        DATA_FORMAT_R16_UNORM = 56,
        DATA_FORMAT_R16_UINT = 57,
        DATA_FORMAT_R16_SNORM = 58,
        DATA_FORMAT_R16_SINT = 59,
        DATA_FORMAT_R8_TYPELESS = 60,
        DATA_FORMAT_R8_UNORM = 61,
        DATA_FORMAT_R8_UINT = 62,
        DATA_FORMAT_R8_SNORM = 63,
        DATA_FORMAT_R8_SINT = 64,
        DATA_FORMAT_A8_UNORM = 65,
        DATA_FORMAT_R1_UNORM = 66,
        DATA_FORMAT_R9G9B9E5_SHAREDEXP = 67,
        DATA_FORMAT_R8G8_B8G8_UNORM = 68,
        DATA_FORMAT_G8R8_G8B8_UNORM = 69,
        DATA_FORMAT_BC1_TYPELESS = 70,
        DATA_FORMAT_BC1_UNORM = 71,
        DATA_FORMAT_BC1_UNORM_SRGB = 72,
        DATA_FORMAT_BC2_TYPELESS = 73,
        DATA_FORMAT_BC2_UNORM = 74,
        DATA_FORMAT_BC2_UNORM_SRGB = 75,
        DATA_FORMAT_BC3_TYPELESS = 76,
        DATA_FORMAT_BC3_UNORM = 77,
        DATA_FORMAT_BC3_UNORM_SRGB = 78,
        DATA_FORMAT_BC4_TYPELESS = 79,
        DATA_FORMAT_BC4_UNORM = 80,
        DATA_FORMAT_BC4_SNORM = 81,
        DATA_FORMAT_BC5_TYPELESS = 82,
        DATA_FORMAT_BC5_UNORM = 83,
        DATA_FORMAT_BC5_SNORM = 84,
        DATA_FORMAT_B5G6R5_UNORM = 85,
        DATA_FORMAT_B5G5R5A1_UNORM = 86,
        DATA_FORMAT_B8G8R8A8_UNORM = 87,
        DATA_FORMAT_B8G8R8X8_UNORM = 88,
        DATA_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
        DATA_FORMAT_B8G8R8A8_TYPELESS = 90,
        DATA_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
        DATA_FORMAT_B8G8R8X8_TYPELESS = 92,
        DATA_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
        DATA_FORMAT_BC6H_TYPELESS = 94,
        DATA_FORMAT_BC6H_UF16 = 95,
        DATA_FORMAT_BC6H_SF16 = 96,
        DATA_FORMAT_BC7_TYPELESS = 97,
        DATA_FORMAT_BC7_UNORM = 98,
        DATA_FORMAT_BC7_UNORM_SRGB = 99,
        DATA_FORMAT_AYUV = 100,
        DATA_FORMAT_Y410 = 101,
        DATA_FORMAT_Y416 = 102,
        DATA_FORMAT_NV12 = 103,
        DATA_FORMAT_P010 = 104,
        DATA_FORMAT_P016 = 105,
        DATA_FORMAT_420_OPAQUE = 106,
        DATA_FORMAT_YUY2 = 107,
        DATA_FORMAT_Y210 = 108,
        DATA_FORMAT_Y216 = 109,
        DATA_FORMAT_NV11 = 110,
        DATA_FORMAT_AI44 = 111,
        DATA_FORMAT_IA44 = 112,
        DATA_FORMAT_P8 = 113,
        DATA_FORMAT_A8P8 = 114,
        DATA_FORMAT_B4G4R4A4_UNORM = 115,

        DATA_FORMAT_P208 = 130,
        DATA_FORMAT_V208 = 131,
        DATA_FORMAT_V408 = 132,

        DATA_FORMAT_FORCE_UINT = 0xffffffff
    } ;

    enum GHIInputType
    {
        PER_VERTEX_DATA = 0,
        PER_INSTANCE_DATA = 1
    };

    struct GHIInputElementInfo
    {
        const char *SemanticName;
        int SemanticIndex;
        GHIDataFormat Format;
        int InputSlot;
        int AlignedByteOffset;
        GHIInputType InputSlotClass;
        int InstanceDataStepRate;
    };

	class IGHIComputeCommandCotext
	{
	public:
        virtual GHIBuffer*  CreateConstBuffer(int size, const void* initData) = 0;
        virtual GHIBuffer*  CreateVertexBuffer(int size, const void* initData) = 0;
        virtual GHIBuffer*  CreateIndexBuffer(int size, const void* initData) = 0;
        virtual GHITexture* CreateTexture(std::string filename) = 0;
        virtual GHITexture* CreateTextureByAnother(GHITexture * tex) = 0;

        virtual void SetRenderTarget(int num, GHITexture **colorBuffers, GHITexture *depthBuffer ) = 0;
        virtual void ClearRenderTarget(int num, GHITexture **colorBuffers, float*clearValue = nullptr ) = 0;
        virtual void ClearDepthStencil(GHITexture *depthBuffers, float depth, int stencil, int flag) = 0;

		virtual void UpdateBuffer(GHIBuffer*buffer, void* data, int size) = 0;
        virtual void SetShaderResource(GHITexture *resource, int slot, GHISRVParam view,EShaderStage stage = EShaderStage::CS) = 0;
        virtual void SetShaderResource(GHITexture *resource, int slot, GHIUAVParam view,EShaderStage stage = EShaderStage::CS) = 0;
        virtual void SetConstBuffer(GHIBuffer *resource, int slot, GHIShader *shader = nullptr) = 0;
        virtual GHISampler* CreateSampler(const GHISamplerDesc  &desc) = 0;
        virtual void SetSampler(GHISampler *resource, int slot, EShaderStage stage) = 0;

		virtual void CopyTexture(GHITexture *dst, GHITexture *src) = 0;
		virtual void Dispatch(int nX, int nY, int nZ) = 0;
		virtual void SetViewport(GHIViewport viewport) = 0;
		virtual void Draw(int count, int offset) = 0;
		virtual void DrawIndexed(int count, int startIndexLocation, int baseIndexLocation) = 0;

		virtual void setPrimitiveTopology(PrimitiveTopology topology) = 0;
		virtual void SetIndexBuffer(GHIBuffer *buffer, GHIIndexType type, int offset) = 0;
		virtual void SetVertexBuffers(int startSlot, int numSlots, GHIBuffer *buffer[], int strides[], int offsets[]) = 0;

        virtual GHIVertexLayout* CreateVertextLayout(const std::vector<GHIInputElementInfo>& vertexFormats, GHIVertexShader *vs) = 0;
        virtual GHIVertexShader*  CreateVertexShader(std::string file, std::string entrypoint) = 0;
        virtual GHIPixelShader*   CreatePixelShader(std::string file, std::string entrypoint) = 0;
        virtual GHIShader* CreateComputeShader(std::string file) = 0;
        virtual GHIShader* CreateShader(std::string file) = 0;
        virtual void SetShader(GHIShader* shader) = 0;
        virtual void SetVertexLayout(GHIVertexLayout* shader) = 0;

        GHIShader* GetComputeShader(std::string file);
	};

}