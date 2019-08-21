//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "FDX11GHIShaderProgram.h" 

namespace GHI
{
    void FDX11GHIShaderProgram::Init(IGHIComputeCommandCotext *commandcontext)
    {
        vs = commandcontext->CreateVertexShader(shaderFile,"VS");
        ps = commandcontext->CreatePixelShader(shaderFile, "PS");
    }


    void FDX11GHIShaderProgram::Update(const Camera &camera, IGHIComputeCommandCotext *commandcontext)
    {

    }

    void FDX11GHIShaderProgram::Apply(const Mesh &model, IGHIComputeCommandCotext *commandcontext)
    {
        const std::vector<GHIInputElementInfo>& vertexFormats = model.VertexElements();
        GHIVertexLayout* vertexLayout = commandcontext->CreateVertextLayout(vertexFormats, vs);
        if (vertexLayout == nullptr)
        {
            // mismatch
            return;
        }

        commandcontext->SetVertexLayout(vertexLayout);
        commandcontext->SetShader(vs);
        commandcontext->SetShader(ps);
        model.Render(commandcontext);
    }

}