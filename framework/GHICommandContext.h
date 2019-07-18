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
	
	class GHIComputeCommandCotext
	{
	public:
		virtual void SetShaderResource(int slot) = 0;
	};
}