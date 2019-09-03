//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

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
        virtual void Update(const UserData&camera, IGHIComputeCommandCotext *commandcontext) override;
        virtual void Apply(const Mesh &model, IGHIComputeCommandCotext *commandcontext) override;
    };
}