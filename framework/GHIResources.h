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

	class GHIResourceView
	{
	private:
		GHIResource *resource;

	public:

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

}