//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "GHICommandContext.h" 
#include "ShaderCache.h" 

namespace SimpleFramework
{

	ShaderCache *gShaderCache = nullptr;

	GHIShader* IGHIComputeCommandCotext::GetComputeShader(std::string file)
	{
		return gShaderCache ? gShaderCache->GetComputerShader(file) : nullptr;
	}
}