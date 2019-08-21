//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "myCamera.h" 
#include "Model.h" 
#include "GHIResources.h" 
#include "GHICommandContext.h" 

namespace GHI
{
    struct  UniformParam 
    {
        virtual void Init(IGHIComputeCommandCotext *commandcontext, GHIBuffer **constBuffer) = 0;
        virtual void Update(const Camera &camera, IGHIComputeCommandCotext *commandcontext, GHIBuffer *constBuffer) = 0;
    };

	class GHIShaderProgram
	{
    protected:
        std::string shaderFile;
        GHIVertexLayout *vertexLayout = nullptr;
        GHIVertexShader *vs = nullptr;
        GHIPixelShader  *ps = nullptr;

        void loadShaderProgram(IGHIComputeCommandCotext *commandContext)
        {
            std::ifstream in(shaderFile);
            std::string str;
            while (std::getline(in, str))
            {
                if (str.size() <= 0)
                {
                    break;
                }
                std::vector<std::string> symbols = Split(str);
                if (symbols[1] == "VS")
                {
                    vs = commandContext->CreateVertexShader(shaderFile, symbols[2]);
                }
                else if (symbols[1] == "PS")
                {
                    ps = commandContext->CreatePixelShader(shaderFile, symbols[2]);
                }
            }
            in.close();
        }

    public:

        GHIShaderProgram(std::string file)
            :shaderFile(file)
        { }

        virtual ~GHIShaderProgram()
        {
			delete vertexLayout;
			delete vs;
			delete ps;
        }

        virtual void Init(IGHIComputeCommandCotext *commandcontext)
        {
            loadShaderProgram(commandcontext);
        }

        virtual void Update(const Camera &camera, IGHIComputeCommandCotext *commandcontext) = 0;

        virtual void Apply(const Mesh &model, IGHIComputeCommandCotext *commandcontext)
        {
            if (vertexLayout == nullptr)
            {
                vertexLayout = commandcontext->CreateVertextLayout(model.VertexElements(), vs);
            }
			commandcontext->SetVertexLayout(vertexLayout);
		    commandcontext->SetShader(vs);
		    commandcontext->SetShader(ps);
            model.Render(commandcontext);
        }
	};
    
    struct  DepthOnlyUniform:public UniformParam
    {
        struct alignas(16) BufferStruct
        {
            Float4x4 WorldViewProjection;
        };

        virtual void Init(IGHIComputeCommandCotext *commandcontext, GHIBuffer **constBuffer) override
        {
            *constBuffer = commandcontext->CreateConstBuffer(sizeof(BufferStruct), nullptr);
        }

        virtual void Update(const Camera &camera, IGHIComputeCommandCotext *commandcontext, GHIBuffer *constBuffer) override
        {
            BufferStruct vsParam;
            vsParam.WorldViewProjection = Float4x4::Transpose(camera.ViewProjectionMatrix());
            commandcontext->UpdateBuffer(constBuffer, &vsParam, sizeof(vsParam));
        }
    };

    class DepthOnlyShader : public GHIShaderProgram
    {
        UniformParam *paramStruct = nullptr;
	    GHIBuffer    *paramBuffer = nullptr;

    public:
        DepthOnlyShader(std::string file = "..\\data\\rawDepth.hlsl")
            : GHIShaderProgram(file)
        {

        }

        ~DepthOnlyShader()
        {
            delete paramStruct;
            delete paramBuffer;
        }

        virtual void Init(IGHIComputeCommandCotext *commandcontext) override
        {
            GHIShaderProgram::Init(commandcontext);
            paramStruct = new DepthOnlyUniform;
            paramStruct->Init(commandcontext, &paramBuffer);
        }

        virtual void Update(const Camera &camera, IGHIComputeCommandCotext *commandcontext) override
        {
            paramStruct->Update(camera, commandcontext, paramBuffer);
		    commandcontext->SetConstBuffer(paramBuffer, 0, vs);
        }

    };


    struct  NoLightingUniform :public UniformParam
    {
        struct alignas(16) BufferStruct
        {
            Float4x4 World;
            Float4x4 ViewProjection;
            Float3  EyePos;
        };

        virtual void Init(IGHIComputeCommandCotext *commandcontext, GHIBuffer **constBuffer) override
        {
            *constBuffer = commandcontext->CreateConstBuffer(sizeof(BufferStruct), nullptr);
        }

        virtual void Update(const Camera &camera, IGHIComputeCommandCotext *commandcontext, GHIBuffer *constBuffer) override
        {
            Float4x4 World;
            World.SetTranslation(Float3(2, 0, 0));
            BufferStruct vsParam;
            vsParam.World = Float4x4::Transpose(World);
            vsParam.ViewProjection = Float4x4::Transpose(camera.ViewProjectionMatrix());
            vsParam.EyePos = camera.Position();
            commandcontext->UpdateBuffer(constBuffer, &vsParam, sizeof(vsParam));
        }
    };

    class  NoLightingShader : public GHIShaderProgram
    {
        UniformParam *paramStruct = nullptr;
        GHIBuffer    *paramBuffer = nullptr;

    public:
        NoLightingShader(std::string file = "..\\data\\rawView.hlsl")
            : GHIShaderProgram(file)
        {

        }

        ~NoLightingShader()
        {
            delete paramStruct;
            delete paramBuffer;
        }

        virtual void Init(IGHIComputeCommandCotext *commandcontext) override
        {
            GHIShaderProgram::Init(commandcontext);
            paramStruct = new NoLightingUniform;
            paramStruct->Init(commandcontext, &paramBuffer);
        }

        virtual void Update(const Camera &camera, IGHIComputeCommandCotext *commandcontext) override
        {
            paramStruct->Update(camera, commandcontext, paramBuffer);
            commandcontext->SetConstBuffer(paramBuffer, 0, vs);
        }

    };
}