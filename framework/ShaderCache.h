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
		void AddGraphicShader(std::string name, GHIShader* shader);
        void EnumCache();

		GHIShader* operator[](std::string name)
		{
			return mShaders[name];
		}

		GHIShader* GetComputerShader(std::string file)
		{

			for (auto it = mComputeShaders.begin(); it != mComputeShaders.end(); ++it)
			{
			
				if ((*it)->info.shaderfile == file)
					return *it;
			}
			return nullptr;
		}

    private:
        void buildCache();

        void release(void)
        {
            mShaders.clear();
        }
    };

}