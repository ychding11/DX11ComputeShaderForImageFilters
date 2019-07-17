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
	class GHIResource
	{

	};

	class GHITexture :public GHIResource
	{

		virtual void LoadFromFile(std::string filename) = 0;
	};

	class GHIBuffer :public GHIResource
	{

		virtual void Update(void* data, int size) = 0;
	};

// Common

}