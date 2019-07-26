//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "GHIResources.h"
#include "GHICommandContext.h"

#include <unordered_map>

namespace SimpleFramework
{

    class ShaderCache
	{
        IGHIComputeCommandCotext *commandContext = nullptr;

		std::vector<std::string> mShaderFiles;
        std::unordered_map<std::string, GHIShader*> mShaders;
        std::vector< GHIShader*> mComputeShaders;
        std::vector< GHIShader*>::iterator  mCurIt;

    public:

        ShaderCache(IGHIComputeCommandCotext *context)
            : commandContext(context)
        {
        }

        ShaderCache(IGHIComputeCommandCotext *context, const std::vector<std::string> &shaders)
            : commandContext(context)
			, mShaderFiles(shaders)
        {
            buildCache();
			enumCache();
        }

        virtual ~ShaderCache()
        {
            release();
        }

		GHIShader* Current() const
		{
			return *mCurIt;
		}

		void Next()
		{
			mCurIt + 1 == mComputeShaders.end() ?  mCurIt = mComputeShaders.begin() : ++mCurIt;
		}

		void Prev()
		{
			mCurIt == mComputeShaders.begin() ?  mCurIt = mComputeShaders.end() - 1 : --mCurIt;
		}

		void InitComputeCache(const std::vector<std::string> &files);



    private:
        void buildCache();
        void enumCache();

        void release(void)
        {
            mShaders.clear();
        }
    };

}