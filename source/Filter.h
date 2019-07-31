/*
 *
 * Header Header
 *
 * Copyright (C) 2014-2015  Yaochuang Ding - <ych_ding@163.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to 
 * deal in the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions, and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution, and in the same 
 *    place and form as other copyright, license and disclaimer information.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 *
 */
#ifndef FILTER_H_
#define FILTER_H_

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <vector>

#include "GHIResources.h"
#include "GHICommandContext.h"

class FilterParam
{
public:
	std::string name;
	SimpleFramework::GHITexture *res;

	SimpleFramework::GHITexture * operator() ()
	{
		return res;
	}
};

class IutputParam : public FilterParam
{
};
class OutputParam : public FilterParam
{
};

class Filter
{
public:
	std::vector<FilterParam*> mInputs;
	std::vector<FilterParam*> mOutputs;
	std::string mShaderFile;
	std::string mDescription = "an image filter";

	Filter(std::string shaderFile)
		: mShaderFile(mShaderFile)
	{

	}

	virtual void UpdateUI() = 0;
	virtual void Active(SimpleFramework::IGHIComputeCommandCotext *commandContext) = 0;

};

class BilaterialFilter : public Filter
{
	SimpleFramework::GHIShader* computeShader = nullptr;
	SimpleFramework::GHIBuffer* constBuffer = nullptr;

	BilaterialFilter(std::string filename)
		: Filter(filename)
	{

	}
	virtual void UpdateUI() override
	{

	}

	virtual void Active(SimpleFramework::IGHIComputeCommandCotext *commandContext) override
	{
		//INFO("active compute shader: [%s]", computeShader->info.shaderfile.c_str());
		//SimpleFramework::GHIUAVParam uav;
		int imageWidth = (*mInputs[0])()->width;
		int imageHeight = (*mOutputs[0])()->height;
		commandContext->SetConstBuffer(constBuffer, 0);
		commandContext->SetShader(computeShader);
		commandContext->SetShaderResource((*mInputs[0])(), 0, SimpleFramework::GHISRVParam());
		commandContext->SetShaderResource((*mOutputs[0])(), 0, SimpleFramework::GHIUAVParam());
		commandContext->Dispatch((imageWidth + 31) / 32, (imageHeight + 31) / 32, 1);
	}
};

#endif /* FILTER_H_*/
