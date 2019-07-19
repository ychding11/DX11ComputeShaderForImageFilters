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
    class GHIResourceView;
	class GHIResource
	{
    public:
        GHIResourceView * view;
        GHIResource() : view(nullptr)
        {
        }
	};


    struct GHIUAVParam
    {

    };
    struct GHISRVParam
    {

    };
    struct GHIRTVParam
    {

    };


	class GHIResourceView
	{
	private:
		GHIResource *resource;

	public:
        GHIResourceView(GHIResource *res)
            :resource(res)
        {

        }
        virtual void CreateRTV(const GHIRTVParam &) = 0;
        virtual void CreateSRV(const GHISRVParam &) = 0;
        virtual void CreateUAV(const GHIUAVParam &) = 0;
	};

    class GHIRenderTargetView :public GHIResourceView
    {

    };

    class GHIShaderResourceView :public GHIResourceView
    {

    };

    class GHIUnorderedAccessView :public GHIResourceView
    {

    };

	class GHITexture :public GHIResource
	{
	public:
		virtual void LoadFromFile(std::string filename) = 0;
	};

	class GHIBuffer :public GHIResource
	{
	public:
		virtual void Update(void* data, int size) = 0;
	};

// Common
    template<class T>
    struct TD3D11ResourceTraits
    {
    };
    template<>
    struct TD3D11ResourceTraits<GHITexture>
    {
        typedef int  TConcreteType;
    };
    template<>
    struct TD3D11ResourceTraits<GHIBuffer>
    {
        typedef int TConcreteType;
    };

    #define FORCEINLINE __forceinline									/* Force code to be inline */

    template<typename TRHIType>
    static FORCEINLINE typename TD3D11ResourceTraits<TRHIType>::TConcreteType* ResourceCast(TRHIType* Resource)
    {
        return static_cast<typename TD3D11ResourceTraits<TRHIType>::TConcreteType*>(Resource);
    }

}