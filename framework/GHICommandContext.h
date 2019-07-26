//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "GHIResources.h" 

namespace SimpleFramework
{
    typedef float               FLOAT;

    enum EResourceView
    {
        RTV,
        UAV,
        SRV,
        NUM
    };

    enum EShaderStage
    {
        VS,
        PS,
        CS,
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

	class IGHIComputeCommandCotext
	{
	public:
        virtual GHIBuffer*  CreateConstBuffer(int size, const void* initData) = 0;
        virtual GHITexture* CreateTexture(std::string filename) = 0;
        virtual GHITexture* CreateTextureByAnother(GHITexture * tex) = 0;

		virtual void UpdateBuffer(GHIBuffer*buffer, void* data, int size) = 0;
        virtual void SetShaderResource(GHITexture *resource, int slot, GHISRVParam view,EShaderStage stage = EShaderStage::CS) = 0;
        virtual void SetShaderResource(GHITexture *resource, int slot, GHIUAVParam view,EShaderStage stage = EShaderStage::CS) = 0;
        virtual void SetConstBuffer(GHIBuffer *resource, int slot) = 0;
        virtual GHISampler* CreateSampler(const GHISamplerDesc  &desc) = 0;
        virtual void SetSampler(GHISampler *resource, int slot, EShaderStage stage) = 0;

		virtual void CopyTexture(GHITexture *dst, GHITexture *src) = 0;
		virtual void Dispatch(int nX, int nY, int nZ) = 0;
		virtual void SetViewport(GHIViewport viewport) = 0;
		virtual void Draw(int count, int offset) = 0;
	};
}