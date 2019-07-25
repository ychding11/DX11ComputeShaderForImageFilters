//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include <string>
#include <d3d11.h>
#include "GHIResources.h" 
#include "DXassert.h" 
#include "Utility.h" 

namespace SimpleFramework
{
    // Safe Release Function
    template <class T>
    void DXRelease(T *&ppT)
    {
        if (ppT)
        {
            ppT->Release();
            ppT = nullptr;
        }
    }

    class FDX11GHIRenderTargetView : public GHIRenderTargetView
    {
		ID3D11RenderTargetView *raw;

    };

    class FDX11GHIShaderResourceView: public GHIShaderResourceView
    {
		ID3D11ShaderResourceView *raw;
    };

    class FDX11GHIUnorderedAccessView: public GHIUnorderedAccessView
    {
		ID3D11UnorderedAccessView *raw;
    };

	class FDX11GHIResourceView: public IGHIResourceView
	{
	private:
		GHITexture *resource;

	public:
        FDX11GHIResourceView(GHITexture *res)
			: resource(res)
        {

        }
        virtual void CreateRTV(const GHIRTVParam &) override;
        virtual void CreateSRV(const GHISRVParam &) override;
        virtual void CreateUAV(const GHIUAVParam &) override;
	};

	class FDX11GHITexture: public GHITexture
	{
	public:

		ID3D11Texture2D *rawTexture = nullptr;
		ID3D11ShaderResourceView *rawSRV = nullptr;
		ID3D11UnorderedAccessView *rawUAV = nullptr;
		ID3D11RenderTargetView *rawRTV = nullptr;

		FDX11GHITexture(ID3D11Texture2D *tex)
			: rawTexture(tex)
		{
			view = new FDX11GHIResourceView(this);
            list.push_back(this);
		}
		FDX11GHITexture(std::string filename)
		{
			LoadFromFile(filename);
			view = new FDX11GHIResourceView(this);
            list.push_back(this);
		}
        virtual void release() override
        {
            WriteLog("Begin, DXRelease() texture");
            DXRelease(rawTexture);
            DXRelease(rawSRV);
            DXRelease(rawUAV);
            DXRelease(rawRTV);
        }

		void LoadFromFile(std::string filename);
	};

	class FDX11GHIBuffer: public GHIBuffer 
	{
	public:
	public:
		ID3D11Buffer* rawBuffer = nullptr;
		FDX11GHIBuffer(ID3D11Buffer *buffer)
            :rawBuffer(buffer)
		{
            list.push_back(this);
		}
        virtual void release() override
        {
            WriteLog("Begin, DXRelease() buffer");
            DXRelease(rawBuffer);
        }
		virtual void Update(void* data, int size) override;
	};

	class FDX11GHISampler : public GHISampler
	{
	public:
		ID3D11SamplerState *rawSampler = nullptr;
		FDX11GHISampler(ID3D11SamplerState *sampler)
			:rawSampler(sampler)
		{
            list.push_back(this);
		}
        virtual void release() override
        {
            WriteLog("Begin, DXRelease(), sampler");
            DXRelease(rawSampler);
            AssertMsg_(rawSampler==nullptr, "Fault, DXRelease()");
        }
	};

    // Cast
    template<class T>
    struct TD3D11ResourceTraits
    {
    };

    template<>
    struct TD3D11ResourceTraits<GHITexture>
    {
        typedef FDX11GHITexture  TConcreteType;
    };

    template<>
    struct TD3D11ResourceTraits<GHIBuffer>
    {
        typedef FDX11GHIBuffer TConcreteType;
    };

    template<>
    struct TD3D11ResourceTraits<GHISampler>
    {
        typedef FDX11GHISampler TConcreteType;
    };

    #define FORCEINLINE __forceinline									/* Force code to be inline */

    template<typename TRHIType>
    static FORCEINLINE typename TD3D11ResourceTraits<TRHIType>::TConcreteType* ResourceCast(TRHIType* Resource)
    {
        return static_cast<typename TD3D11ResourceTraits<TRHIType>::TConcreteType*>(Resource);
    }

}