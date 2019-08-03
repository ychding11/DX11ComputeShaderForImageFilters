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

#include "imgui.h"
#include "Utils.h"
#include "GHIResources.h"
#include "GHICommandContext.h"

class FilterParam
{
public:
	std::string name;
	GHI::GHITexture *res;

	FilterParam(std::string parname, GHI::GHITexture *tex)
		: name(parname)
		, res(tex)
	{}

	GHI::GHITexture * operator() ()
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
protected:
	std::vector<FilterParam*> mInputs;
	std::vector<FilterParam*> mOutputs;
	GHI::GHISampler *sampler = nullptr;
	GHI::GHIShader* computeShader = nullptr;

public:
	std::string mShaderFile;
	std::string mDescription = "an image filter";

	Filter(std::string shaderFile)
		: mShaderFile(shaderFile)
	{

	}

	void setSampler(GHI::GHISampler *samp)
	{
		sampler = samp;
	}
	void addInput(GHI::GHITexture *res)
	{
		mInputs.push_back(new FilterParam("input image",res));
	}
	void addOutput(GHI::GHITexture *res)
	{
		mOutputs.push_back(new FilterParam("output image",res));
	}

	virtual void Init(GHI::IGHIComputeCommandCotext *commandContext)
	{
		computeShader = commandContext->GetComputeShader(mShaderFile);
	}

	virtual void UpdateUI(GHI::IGHIComputeCommandCotext *commandContext)
	{

	}
	virtual void Active(GHI::IGHIComputeCommandCotext *commandContext)
	{
		DEBUG("active compute shader: [%s]", computeShader->info.shaderfile.c_str());
		int imageWidth = (*mInputs[0])()->width;
		int imageHeight = (*mInputs[0])()->height;

		commandContext->SetSampler(sampler, 0, GHI::EShaderStage::CS);
		commandContext->SetShader(computeShader);
		commandContext->SetShaderResource((*mInputs[0])(), 0, GHI::GHISRVParam());
		commandContext->SetShaderResource((*mOutputs[0])(), 0, GHI::GHIUAVParam());
		commandContext->Dispatch((imageWidth + 31) / 32, (imageHeight + 31) / 32, 1);
	}
};

class BilaterialFilter : public Filter
{
	GHI::GHIBuffer* constBuffer = nullptr;
    int windowWdith = 5;
public:
	BilaterialFilter(std::string filename = "..\\effects\\test.hlsl")
		: Filter(filename)
	{
        
	}

	struct alignas(16) FilterSize
	{
		unsigned int wSize;
	};

    virtual void Init(GHI::IGHIComputeCommandCotext *commandContext) override
    {
        // Generate const buffer definition & create Const buffer
		FilterSize cb = { 15 };
		constBuffer = commandContext->CreateConstBuffer(sizeof(cb), &cb);
		commandContext->SetConstBuffer(constBuffer, 0);
		computeShader = commandContext->GetComputeShader(mShaderFile);
    }

	virtual void UpdateUI(GHI::IGHIComputeCommandCotext *commandContext) override
	{
        ImGui::Begin("Bilaterial UI");
        ImGui::Text("Parameter tweak."); // Display some text (you can use a format strings too)
        ImGui::SameLine();
        if (ImGui::SliderInt("Filter Window",&windowWdith, 3, 17))
        {
            windowWdith & 0x1 ? windowWdith : windowWdith += 1;
			DEBUG("Filter Size:%d", windowWdith);
        }
        ImGui::End();
	}

	virtual void Active(GHI::IGHIComputeCommandCotext *commandContext) override
	{
		DEBUG("active compute shader: [%s]", computeShader->info.shaderfile.c_str());
		FilterSize data;
		data.wSize = windowWdith;
		commandContext->UpdateBuffer(constBuffer, &data, sizeof(data));

		int imageWidth = (*mInputs[0])()->width;
		int imageHeight = (*mInputs[0])()->height;
		commandContext->SetSampler(sampler, 0, GHI::EShaderStage::CS);
		commandContext->SetConstBuffer(constBuffer, 1);
		commandContext->SetShader(computeShader);
		commandContext->SetShaderResource((*mInputs[0])(), 0, GHI::GHISRVParam());
		commandContext->SetShaderResource((*mOutputs[0])(), 0, GHI::GHIUAVParam());
		commandContext->Dispatch((imageWidth + 31) / 32, (imageHeight + 31) / 32, 1);
	}
};

#endif /* FILTER_H_*/
