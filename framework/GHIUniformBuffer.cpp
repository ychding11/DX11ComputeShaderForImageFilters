//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "GHIResources.h" 
#include "GHIUniformBuffer.h" 

namespace SimpleFramework
{
    std::list<FUniformBufferStruct*>& FUniformBufferStruct::GetStructList()
    {
        static std::list<FUniformBufferStruct*> GUniformStructList;
        return GUniformStructList;

    }
    /** Speed up finding the uniform buffer by its name */
    std::unordered_map<std::string, FUniformBufferStruct*>& FUniformBufferStruct::GetNameStructMap()
    {
        static 	std::unordered_map<std::string, FUniformBufferStruct*> GlobalNameStructMap;
        return GlobalNameStructMap;
    }

    /** Initialization constructor. */
    FUniformBufferStruct::FUniformBufferStruct(const std::string InLayoutName, const char* InStructTypeName, const char* InShaderVariableName, ConstructUniformBufferParameterType InConstructRef, uint32 InSize, const std::vector<FMember>& InMembers, bool bRegisterForAutoBinding)
        : StructTypeName(InStructTypeName)
        , ShaderVariableName(InShaderVariableName)
        , ConstructUniformBufferParameterRef(InConstructRef)
        , Size(InSize)
        , bLayoutInitialized(false)
        , Layout(InLayoutName)
        , Members(InMembers)
    {
        if (bRegisterForAutoBinding)
        {
            GetStructList().push_back(this);
            std::string StrutTypeFName(StructTypeName);
            GetNameStructMap().insert({ StrutTypeFName, this });
        }
        else
        {
            // We cannot initialize the layout during global initialization, since we have to walk nested struct members.
            // Structs created during global initialization will have bRegisterForAutoBinding==false, and are initialized during startup.
            // Structs created at runtime with bRegisterForAutoBinding==true can be initialized now.
            InitializeLayout();
        }
    }

    void FUniformBufferStruct::InitializeLayout()
    {
#if 0
        check(!bLayoutInitialized);
        Layout.ConstantBufferSize = Size;

        TArray<FUniformBufferMemberAndOffset> MemberStack;
        MemberStack.Reserve(Members.Num());

        for (int32 MemberIndex = 0; MemberIndex < Members.Num(); MemberIndex++)
        {
            MemberStack.Push(FUniformBufferMemberAndOffset(Members[MemberIndex], 0));
        }

        for (int32 i = 0; i < MemberStack.Num(); ++i)
        {
            const FMember& CurrentMember = MemberStack[i].Member;
            bool bIsResource = IsUniformBufferResourceType(CurrentMember.GetBaseType());

            if (bIsResource)
            {
                Layout.Resources.Add(CurrentMember.GetBaseType());
                const uint32 AbsoluteMemberOffset = CurrentMember.GetOffset() + MemberStack[i].StructOffset;
                check(AbsoluteMemberOffset < (1u << (Layout.ResourceOffsets.GetTypeSize() * 8)));
                Layout.ResourceOffsets.Add(AbsoluteMemberOffset);
            }

            const FUniformBufferStruct* MemberStruct = CurrentMember.GetStruct();

            if (MemberStruct)
            {
                int32 AbsoluteStructOffset = CurrentMember.GetOffset() + MemberStack[i].StructOffset;

                for (int32 StructMemberIndex = 0; StructMemberIndex < MemberStruct->Members.Num(); StructMemberIndex++)
                {
                    FMember StructMember = MemberStruct->Members[StructMemberIndex];
                    MemberStack.Insert(FUniformBufferMemberAndOffset(StructMember, AbsoluteStructOffset), i + 1 + StructMemberIndex);
                }
            }
        }

        bLayoutInitialized = true;
#endif
    }


}