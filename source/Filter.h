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
#include "ImNodes.h"
#include "ImNodesEz.h"
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
	std::string mShaderFile;
	std::string mDescription = "an image filter";

public:

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
		mInputs.size() <=0 ? mInputs.push_back(new FilterParam("input image",res)) : mInputs[0] = new FilterParam("input image",res);
	}
	void addOutput(GHI::GHITexture *res)
	{
		mOutputs.size() <= 0 ? mOutputs.push_back(new FilterParam("output image",res)) : mOutputs[0] = new FilterParam("output image",res);
	}

	virtual void Init(GHI::IGHIComputeCommandCotext *commandContext)
	{
		computeShader = commandContext->GetComputeShader(mShaderFile);
	}

	virtual void UpdateUI(GHI::IGHIComputeCommandCotext *commandContext)
	{
        static ImNodes::CanvasState canvas;

        if (ImGui::Begin("ImNodes", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            ImNodes::BeginCanvas(&canvas);

            struct Node
            {
                ImVec2 pos{};
                bool selected{};
                ImNodes::Ez::SlotInfo inputs[1];
                ImNodes::Ez::SlotInfo outputs[1];
            };

            static Node nodes[3] =
            {
            { { 50, 100 }, false,{ { "In", 1 } },{ { "Out", 1 } } },
            { { 250, 50 }, false,{ { "In", 1 } },{ { "Out", 1 } } },
            { { 250, 100 }, false,{ { "In", 1 } },{ { "Out", 1 } } },
            };

            if (ImNodes::Ez::BeginNode(&nodes[0], "Input Image", &nodes[0].pos, &nodes[0].selected))
            {
                ImNodes::Ez::InputSlots(nodes[0].inputs, 0);
                ImNodes::Ez::OutputSlots(nodes[0].outputs, 1);
                ImNodes::Ez::EndNode();
            }
            if (ImNodes::Ez::BeginNode(&nodes[1], mDescription.c_str(), &nodes[1].pos, &nodes[1].selected))
            {
                ImNodes::Ez::InputSlots(nodes[1].inputs, 1);
                ImNodes::Ez::OutputSlots(nodes[1].outputs, 1);
                ImNodes::Ez::EndNode();
            }
            if (ImNodes::Ez::BeginNode(&nodes[2], "Output Image", &nodes[2].pos, &nodes[2].selected))
            {
                ImNodes::Ez::InputSlots(nodes[2].inputs, 1);
                ImNodes::Ez::OutputSlots(nodes[2].outputs, 0);
                ImNodes::Ez::EndNode();
            }
            ImNodes::Connection(&nodes[1], "In", &nodes[0], "Out");
            ImNodes::Connection(&nodes[2], "In", &nodes[1], "Out");

            ImNodes::EndCanvas();
        }
        ImGui::End();
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
	struct alignas(16) FilterSize
	{
		unsigned int wSize;
	};

	FilterSize data;
	GHI::GHIBuffer* constBuffer = nullptr;
    int windowWdith = 5;
public:
	BilaterialFilter(std::string filename = "..\\effects\\test.hlsl")
		: Filter(filename)
	{
        mDescription = "Bilaterial Filter";
	}

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
        Filter::UpdateUI(commandContext);

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
        if (data.wSize != windowWdith)
        {
            data.wSize = windowWdith;
		    commandContext->UpdateBuffer(constBuffer, &data, sizeof(data));
        }

		int imageWidth = (*mInputs[0])()->width;
		int imageHeight = (*mInputs[0])()->height;
		commandContext->SetSampler(sampler, 0, GHI::EShaderStage::CS);
		commandContext->SetConstBuffer(constBuffer, 0);
		commandContext->SetShader(computeShader);
		commandContext->SetShaderResource((*mInputs[0])(), 0, GHI::GHISRVParam());
		commandContext->SetShaderResource((*mOutputs[0])(), 0, GHI::GHIUAVParam());
		commandContext->Dispatch((imageWidth + 31) / 32, (imageHeight + 31) / 32, 1);
	}

};

    class FishEyeFilter :public Filter
    {
	    struct alignas(16) ImageSize
	    {
		    unsigned int width;
		    unsigned int height;
	    };

	    ImageSize data;
        GHI::GHIBuffer* constBuffer = nullptr;
    public:
        FishEyeFilter(std::string filename = "..\\effects\\fishEye.hlsl")
            : Filter(filename)
        {
            mDescription = "Fish Eye Filter";
        }

        virtual void Init(GHI::IGHIComputeCommandCotext *commandContext) override
        {
            // Generate const buffer definition & create Const buffer
            ImageSize cb = {  };
            constBuffer = commandContext->CreateConstBuffer(sizeof(cb), &cb);
            commandContext->SetConstBuffer(constBuffer, 0);
            computeShader = commandContext->GetComputeShader(mShaderFile);
        }
        virtual void Active(GHI::IGHIComputeCommandCotext *commandContext) override
        {
            DEBUG("active compute shader: [%s]", computeShader->info.shaderfile.c_str());

            int imageWidth = (*mInputs[0])()->width;
            int imageHeight = (*mInputs[0])()->height;
            if (imageWidth != data.width || imageHeight != data.height)
            {
                data.height = imageHeight;
                data.width = imageWidth;
                commandContext->UpdateBuffer(constBuffer, &data, sizeof(data));
            }
            commandContext->SetSampler(sampler, 0, GHI::EShaderStage::CS);
            commandContext->SetConstBuffer(constBuffer, 0);
            commandContext->SetShader(computeShader);
            commandContext->SetShaderResource((*mInputs[0])(), 0, GHI::GHISRVParam());
            commandContext->SetShaderResource((*mOutputs[0])(), 0, GHI::GHIUAVParam());
            commandContext->Dispatch((imageWidth + 31) / 32, (imageHeight + 31) / 32, 1);
        }
    };

    class LensCircleFilter :public Filter
    {
        struct alignas(16) ImageSize
        {
            unsigned int width;
            unsigned int height;
        };

        ImageSize data;
        GHI::GHIBuffer* constBuffer = nullptr;
    public:
        LensCircleFilter(std::string filename = "..\\effects\\lensCircle.hlsl")
            : Filter(filename)
        {
            mDescription = "Lens Circle Filter";
        }

        virtual void Init(GHI::IGHIComputeCommandCotext *commandContext) override
        {
            // Generate const buffer definition & create Const buffer
            ImageSize cb = {};
            constBuffer = commandContext->CreateConstBuffer(sizeof(cb), &cb);
            commandContext->SetConstBuffer(constBuffer, 0);
            computeShader = commandContext->GetComputeShader(mShaderFile);
        }
        virtual void Active(GHI::IGHIComputeCommandCotext *commandContext) override
        {
            DEBUG("active compute shader: [%s]", computeShader->info.shaderfile.c_str());

            int imageWidth = (*mInputs[0])()->width;
            int imageHeight = (*mInputs[0])()->height;
            if (imageWidth != data.width || imageHeight != data.height)
            {
                data.height = imageHeight;
                data.width = imageWidth;
                commandContext->UpdateBuffer(constBuffer, &data, sizeof(data));
            }
            commandContext->SetSampler(sampler, 0, GHI::EShaderStage::CS);
            commandContext->SetConstBuffer(constBuffer, 0);
            commandContext->SetShader(computeShader);
            commandContext->SetShaderResource((*mInputs[0])(), 0, GHI::GHISRVParam());
            commandContext->SetShaderResource((*mOutputs[0])(), 0, GHI::GHIUAVParam());
            commandContext->Dispatch((imageWidth + 31) / 32, (imageHeight + 31) / 32, 1);
        }
    };

    class SwirlFilter :public Filter
    {
        struct alignas(16) ImageSize
        {
            unsigned int width;
            unsigned int height;
        };

        ImageSize data;
        GHI::GHIBuffer* constBuffer = nullptr;
    public:
        SwirlFilter(std::string filename = "..\\effects\\swirl.hlsl")
            : Filter(filename)
        {
            mDescription = "Swirl Filter";
        }

        virtual void Init(GHI::IGHIComputeCommandCotext *commandContext) override
        {
            // Generate const buffer definition & create Const buffer
            ImageSize cb = {};
            constBuffer = commandContext->CreateConstBuffer(sizeof(cb), &cb);
            commandContext->SetConstBuffer(constBuffer, 0);
            computeShader = commandContext->GetComputeShader(mShaderFile);
        }
        virtual void Active(GHI::IGHIComputeCommandCotext *commandContext) override
        {
            DEBUG("active compute shader: [%s]", computeShader->info.shaderfile.c_str());

            int imageWidth = (*mInputs[0])()->width;
            int imageHeight = (*mInputs[0])()->height;
            if (imageWidth != data.width || imageHeight != data.height)
            {
                data.height = imageHeight;
                data.width = imageWidth;
                commandContext->UpdateBuffer(constBuffer, &data, sizeof(data));
            }
            commandContext->SetSampler(sampler, 0, GHI::EShaderStage::CS);
            commandContext->SetConstBuffer(constBuffer, 0);
            commandContext->SetShader(computeShader);
            commandContext->SetShaderResource((*mInputs[0])(), 0, GHI::GHISRVParam());
            commandContext->SetShaderResource((*mOutputs[0])(), 0, GHI::GHIUAVParam());
            commandContext->Dispatch((imageWidth + 31) / 32, (imageHeight + 31) / 32, 1);
        }
    };

#endif /* FILTER_H_*/
