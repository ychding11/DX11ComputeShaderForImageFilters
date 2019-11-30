//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include <camera/myCamera.h>
#include <io/Model.h> 
#include "GHIResources.h" 
#include "GHICommandContext.h" 

namespace GHI
{
    struct UserData
    {
        Camera *camera;
        Float4x4 worldMat;
    };

    struct  UniformParam 
    {
        virtual void Init(IGHIComputeCommandCotext *commandcontext, GHIBuffer **constBuffer) = 0;
        virtual void Update(const UserData &data, IGHIComputeCommandCotext *commandcontext, GHIBuffer *constBuffer) = 0;
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

        virtual void Update(const UserData&camera, IGHIComputeCommandCotext *commandcontext) = 0;

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

        virtual void Update(const UserData &data, IGHIComputeCommandCotext *commandcontext, GHIBuffer *constBuffer) override
        {
            BufferStruct vsParam;
            vsParam.WorldViewProjection = Float4x4::Transpose(data.camera->ViewProjectionMatrix());
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

        virtual void Update(const UserData &data, IGHIComputeCommandCotext *commandcontext) override
        {
            paramStruct->Update(data, commandcontext, paramBuffer);
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

        virtual void Update(const UserData &data, IGHIComputeCommandCotext *commandcontext, GHIBuffer *constBuffer) override
        {
            BufferStruct vsParam;
            vsParam.World = Float4x4::Transpose(data.worldMat);
            vsParam.ViewProjection = Float4x4::Transpose(data.camera->ViewProjectionMatrix());
            vsParam.EyePos = data.camera->Position();
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

        virtual void Update(const UserData &data, IGHIComputeCommandCotext *commandcontext) override
        {
            paramStruct->Update(data, commandcontext, paramBuffer);
            commandcontext->SetConstBuffer(paramBuffer, 0, vs);
        }
    };

    class  RawInstancedShader : public GHIShaderProgram
    {
        UniformParam *paramStruct = nullptr;
        GHIBuffer    *paramBuffer = nullptr;

    public:
        GHIBuffer    *instanceBuffer = nullptr;
        int instanceStride = 0;
        int instanceOffset = 0;
        int instanceCount = 0;

    public:
        RawInstancedShader(std::string file = "..\\data\\rawInstance.hlsl")
            : GHIShaderProgram(file)
        {

        }

        ~RawInstancedShader()
        {
            delete paramStruct;
            delete paramBuffer;
            delete instanceBuffer;
        }

        virtual void Init(IGHIComputeCommandCotext *commandcontext) override
        {
            GHIShaderProgram::Init(commandcontext);
            paramStruct = new NoLightingUniform;
            paramStruct->Init(commandcontext, &paramBuffer);
        }

        virtual void Update(const UserData &data, IGHIComputeCommandCotext *commandcontext) override
        {
            paramStruct->Update(data, commandcontext, paramBuffer);
            commandcontext->SetConstBuffer(paramBuffer, 0, vs);
        }

        virtual void Apply(const Mesh &model, IGHIComputeCommandCotext *commandcontext) override
        {
            if (vertexLayout == nullptr)
            {
                vertexLayout = commandcontext->CreateVertextLayout(model.VertexElements(), vs);
            }
            commandcontext->SetVertexLayout(vertexLayout);
            commandcontext->SetShader(vs);
            commandcontext->SetShader(ps);
            if (instanceBuffer == nullptr)
            {
                model.Render(commandcontext);
            }
            else
            {
                model.RenderInstanced(commandcontext, instanceBuffer, instanceStride, instanceOffset, instanceCount);
            }
        }
    };








    struct  UniformBufferParam
    {
        virtual void Init(IGHIComputeCommandCotext *commandcontext ) = 0;
        virtual void Update(const UserData &data, IGHIComputeCommandCotext *commandcontext) = 0;
        virtual int  ConstBufferCountVS() const = 0;
        virtual int  ConstBufferCountPS() const = 0;
        virtual GHIBuffer* ConstBufferVS(int i) const = 0;
        virtual GHIBuffer* ConstBufferPS(int i) const = 0;
    };


    struct  DirectionalLightUniformBufferParam :public UniformBufferParam
    {
        struct alignas(16) DirectionalLight
        {
            Float3     direction;
            Float3     radiance;
            uint32_t   enabled;
        };

        struct alignas(16) PSConstBuffer
        {
            DirectionalLight lights[3];
        };

        virtual int  ConstBufferCountVS() const override { return 0; }
        virtual int  ConstBufferCountPS() const override { return 1; }
        virtual GHIBuffer* ConstBufferVS(int i)const override 
        {
            return nullptr;
        }
        virtual GHIBuffer* ConstBufferPS(int i) const override 
        {
            if (i != 0)
            {
                return nullptr;
            }
            return psCBuffer;
        }

        virtual void Init(IGHIComputeCommandCotext *commandcontext) override
        {
            psCBuffer = commandcontext->CreateConstBuffer(sizeof(PSConstBuffer), nullptr);
        }

        virtual void Update(const UserData &data, IGHIComputeCommandCotext *commandcontext ) override
        {
            PSConstBuffer psParam;
            commandcontext->UpdateBuffer(psCBuffer, &psParam, sizeof(psParam));
        }

    private:
        GHIBuffer * psCBuffer = nullptr;
    };

    struct  PBRShaderUniformBufferParam :public UniformBufferParam
    {
        struct alignas(16) VSConstBuffer
        {
            Float4x4 World;
            Float4x4 ViewProjection;
        };

        struct alignas(16) PSConstBuffer
        {
            Float3      eyePosition;
            uint32_t    cShadingFlag;
        };

        virtual int  ConstBufferCountVS() const override { return 1; }
        virtual int  ConstBufferCountPS() const override { return 1; }
        virtual GHIBuffer* ConstBufferVS(int i)const override 
        {
            if (i != 0)
            {
                return nullptr;
            }
            return vsCBuffer;
        }
        virtual GHIBuffer* ConstBufferPS(int i) const override 
        {
            if (i != 0)
            {
                return nullptr;
            }
            return psCBuffer;
        }

        virtual void Init(IGHIComputeCommandCotext *commandcontext) override
        {
            vsCBuffer = commandcontext->CreateConstBuffer(sizeof(VSConstBuffer), nullptr);
            psCBuffer = commandcontext->CreateConstBuffer(sizeof(PSConstBuffer), nullptr);
        }

        virtual void Update(const UserData &data, IGHIComputeCommandCotext *commandcontext ) override
        {
            VSConstBuffer vsParam;
            vsParam.World = Float4x4();
            vsParam.ViewProjection = Float4x4::Transpose(data.camera->ViewProjectionMatrix());
            commandcontext->UpdateBuffer(vsCBuffer, &vsParam, sizeof(vsParam));

            PSConstBuffer psParam;
            commandcontext->UpdateBuffer(psCBuffer, &psParam, sizeof(psParam));
        }

    private:
        GHIBuffer * vsCBuffer = nullptr;
        GHIBuffer * psCBuffer = nullptr;

        //std::vector<GHIBuffer*> vsBuffers;
        //std::vector<GHIBuffer*> psBuffers;
    };


    class PBRShader : public GHIShaderProgram
    {
        UniformBufferParam *uniformParam = nullptr;
        UniformBufferParam *lightParam = nullptr;
    public:
        PBRShader(std::string file = "..\\data\\pbr.hlsl")
            : GHIShaderProgram(file)
        {
        }

        ~PBRShader()
        {
            delete uniformParam ;
            delete lightParam ;
        }

        virtual void Init(IGHIComputeCommandCotext *commandcontext) override
        {
            GHIShaderProgram::Init(commandcontext);
            uniformParam = new PBRShaderUniformBufferParam;
            uniformParam->Init(commandcontext);
            lightParam = new DirectionalLightUniformBufferParam;
            lightParam->Init(commandcontext);
        }

        virtual void Update(const UserData &data, IGHIComputeCommandCotext *commandcontext) override
        {
            uniformParam->Update(data, commandcontext);
            lightParam->Update(data, commandcontext);
        }

        virtual void Apply(const Mesh &mesh, IGHIComputeCommandCotext *commandcontext) override
        {
            if (vertexLayout == nullptr)
            {
                vertexLayout = commandcontext->CreateVertextLayout(mesh.VertexElements(), vs);
            }
            commandcontext->SetVertexLayout(vertexLayout);
            commandcontext->SetShader(vs);
            commandcontext->SetShader(ps);
            for (int i = 0; i < uniformParam->ConstBufferCountVS(); ++i)
            {
                commandcontext->SetConstBuffer(uniformParam->ConstBufferVS(i), 0, vs);
            }
            for (int i = 0; i < uniformParam->ConstBufferCountPS(); ++i)
            {
                commandcontext->SetConstBuffer(uniformParam->ConstBufferPS(i), 0, ps);
            }

            for (int i = 0; i < lightParam->ConstBufferCountPS(); ++i)
            {
                commandcontext->SetConstBuffer(lightParam->ConstBufferPS(i), 1, ps);
            }

            {
                mesh.Render(commandcontext);
            }
        }
    };
}