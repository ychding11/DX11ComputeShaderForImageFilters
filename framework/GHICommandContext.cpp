//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "GHICommandContext.h" 
#include "ShaderCache.h" 
#include "Utility.h" 

namespace SimpleFramework
{

	ShaderCache *gShaderCache = nullptr;

	GHIShader* IGHIComputeCommandCotext::GetComputeShader(std::string file)
	{
        if ( FileExtension(file.c_str()) != "hlsl")
        {
            ELOG("shader file extension is NOT qualified.");
        }
		return gShaderCache ? gShaderCache->GetComputerShader(file) : nullptr;
	}
}