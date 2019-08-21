//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

//#include "myCamera.h" 
//#include "Model.h" 
//#include "GHIResources.h" 
//#include "GHICommandContext.h" 

#include "GHIShaderProgram.h" 

namespace GHI
{
    class FDX11GHIShaderProgram : public GHIShaderProgram
    {
    public:
        FDX11GHIShaderProgram(std::string file)
            :GHIShaderProgram(file)
        { }

        virtual void Init(IGHIComputeCommandCotext *commandcontext) override;
        virtual void Update(const Camera &camera, IGHIComputeCommandCotext *commandcontext) override;
        virtual void Apply(const Mesh &model, IGHIComputeCommandCotext *commandcontext) override;
    };
}