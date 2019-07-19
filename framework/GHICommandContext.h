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
	
    enum EResourceView
    {
        RTV,
        UAV,
        SRV,
        NUM
    };
	class GHIComputeCommandCotext
	{
	public:
		virtual void SetShaderResource(GHIResource resource, int slot, EResourceView view) = 0;
		virtual void Dispatch(int nX, int nY, int nZ) = 0;
	};
}