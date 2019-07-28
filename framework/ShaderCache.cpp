//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "ShaderCache.h"
#include "Utility.h"

namespace SimpleFramework
{

	void ShaderCache::InitComputeCache(const std::vector<std::string> &files)
	{
		for (auto it = files.begin(); it != files.end(); ++it)
		{
			std::string filename = *it;
			GHIShader *shader = commandContext->CreateShader(filename);
            mComputeShaders.push_back(shader);
		}
		mCurIt = mComputeShaders.begin();
	}

	void ShaderCache::buildCache()
	{
		for (auto it = mShaderFiles.begin(); it != mShaderFiles.end(); ++it)
		{
			std::string filename = *it;
			GHIShader *shader = commandContext->CreateShader(filename);
#if 0
			auto ret = mShaders.insert( std::pair<std::string, GHIShader*>(filename, shader) );
			if (ret.second == false)
			{
				DLOG("- Shader already cached:", filename.c_str());
			}
#endif
            mComputeShaders.push_back(shader);
		}
		mCurIt = mComputeShaders.begin();
	}

	void ShaderCache::AddGraphicShader(std::string name, GHIShader* shader)
	{
		mShaders[name] = shader;
	}

	void ShaderCache::EnumCache()
	{
		for (auto it = mShaders.begin(); it != mShaders.end(); ++it)
		{
			DLOG("%s --> %s",it->first.c_str(), it->second->str().c_str());
		}

		for (auto it = mComputeShaders.begin(); it != mComputeShaders.end(); ++it)
		{
			DLOG("compute shader --> %s", (*it)->info.shaderfile.c_str());
		}
	}


}