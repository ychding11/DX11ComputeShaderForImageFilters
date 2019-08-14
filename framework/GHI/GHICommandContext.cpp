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

namespace GHI
{

	ShaderCache *gShaderCache = nullptr;

	GHIShader* IGHIComputeCommandCotext::GetComputeShader(std::string file)
	{
        if ( FileExtension(file.c_str()) != "hlsl")
        {
            ELOG("shader file extension is NOT qualified.");
        }
		GHIShader *ret = gShaderCache ? gShaderCache->GetComputerShader(file) : nullptr;
		if (ret == nullptr)
		{
			ret = this->CreateComputeShader(file);
			gShaderCache->AddComputeShader(ret);
		}
		return ret;
	}
}