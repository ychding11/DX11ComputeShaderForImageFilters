//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include <string>
#include <vector>
#include <list>
#include <unordered_map>

typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

namespace SimpleFramework
{
    /** The base type of a value in a uniform buffer. */
    enum EUniformBufferBaseType
    {
        UBMT_INVALID,
        UBMT_BOOL,
        UBMT_INT32,
        UBMT_UINT32,
        UBMT_FLOAT32,
        UBMT_STRUCT,
        UBMT_SRV,
        UBMT_UAV,
        UBMT_SAMPLER,
        UBMT_TEXTURE,

        EUniformBufferBaseType_Num,
        EUniformBufferBaseType_NumBits = 4,
    };
    static_assert(EUniformBufferBaseType_Num <= (1 << EUniformBufferBaseType_NumBits), "EUniformBufferBaseType_Num will not fit on EUniformBufferBaseType_NumBits");



    /** The layout of a uniform buffer in memory. */
    struct GHIUniformBufferLayout
    {
        /** The size of the constant buffer in bytes. */
        uint32 ConstantBufferSize;
        /** Byte offset to each resource in the uniform buffer memory. */
        std::vector<uint16> ResourceOffsets;
        /** The type of each resource (EUniformBufferBaseType). */
        std::vector<uint8> Resources;

        explicit GHIUniformBufferLayout(std::string InName) :
            ConstantBufferSize(0),
            Name(InName),
            Hash(0)
        {
        }

        enum EInit
        {
            Zero
        };

        explicit GHIUniformBufferLayout(EInit) :
            ConstantBufferSize(0),
            Name(),
            Hash(0)
        {
        }

        void CopyFrom(const GHIUniformBufferLayout& Source)
        {
            ConstantBufferSize = Source.ConstantBufferSize;
            ResourceOffsets = Source.ResourceOffsets;
            Resources = Source.Resources;
            Name = Source.Name;
            Hash = Source.Hash;
        }

        const std::string GetDebugName() const { return Name; }

    private:
        // for debugging / error message
        std::string Name;

        uint32 Hash;
    };

    /** Compare two uniform buffer layouts. */
    inline bool operator==(const GHIUniformBufferLayout& A, const GHIUniformBufferLayout& B)
    {
        return A.ConstantBufferSize == B.ConstantBufferSize
            && A.ResourceOffsets == B.ResourceOffsets
            && A.Resources == B.Resources;
    }

    /** A uniform buffer struct. */
    class FUniformBufferStruct
    {
    public:

        /** A member of a uniform buffer type. */
        class FMember
        {
        public:

            /** Initialization constructor. */
            FMember(
                const char* InName,
                const char* InShaderType,
                uint32 InOffset,
                EUniformBufferBaseType InBaseType,
                uint32 InNumRows,
                uint32 InNumColumns,
                uint32 InNumElements,
                const FUniformBufferStruct* InStruct
            )
                : Name(InName)
                , ShaderType(InShaderType)
                , Offset(InOffset)
                , BaseType(InBaseType)
                , NumRows(InNumRows)
                , NumColumns(InNumColumns)
                , NumElements(InNumElements)
                , Struct(InStruct)
            {}

            const char* GetName() const { return Name; }
            const char* GetShaderType() const { return ShaderType; }
            uint32 GetOffset() const { return Offset; }
            EUniformBufferBaseType GetBaseType() const { return BaseType; }
            uint32 GetNumRows() const { return NumRows; }
            uint32 GetNumColumns() const { return NumColumns; }
            uint32 GetNumElements() const { return NumElements; }
            const FUniformBufferStruct* GetStruct() const { return Struct; }

        private:

            const char* Name;
            const char* ShaderType;
            uint32 Offset;
            EUniformBufferBaseType BaseType;
            uint32 NumRows;
            uint32 NumColumns;
            uint32 NumElements;
            const FUniformBufferStruct* Struct;
        };

        typedef class FShaderUniformBufferParameter* (*ConstructUniformBufferParameterType)();

        /** Initialization constructor. */
        FUniformBufferStruct(const std::string InLayoutName, const char* InStructTypeName, const char* InShaderVariableName, ConstructUniformBufferParameterType InConstructRef, uint32 InSize, const std::vector<FMember>& InMembers, bool bRegisterForAutoBinding);

        virtual ~FUniformBufferStruct()
        {
        }

        void InitializeLayout();

        //void AddResourceTableEntries(TMap<FString, FResourceTableEntry>& ResourceTableMap, TMap<FString, uint32>& ResourceTableLayoutHashes) const;

        const char* GetStructTypeName() const { return StructTypeName; }
        const char* GetShaderVariableName() const { return ShaderVariableName; }
        const uint32 GetSize() const { return Size; }
        const GHIUniformBufferLayout& GetLayout() const
        {
            //check(bLayoutInitialized);
            return Layout;
        }
        const std::vector<FMember>& GetMembers() const { return Members; }
        FShaderUniformBufferParameter* ConstructTypedParameter() const { return (*ConstructUniformBufferParameterRef)(); }

        static std::list<FUniformBufferStruct*>& GetStructList();
        /** Speed up finding the uniform buffer by its name */
        static std::unordered_map<std::string, FUniformBufferStruct*>& GetNameStructMap();

        static void InitializeStructs();

    private:
        const char* StructTypeName;
        const char* ShaderVariableName;
        ConstructUniformBufferParameterType ConstructUniformBufferParameterRef;
        uint32 Size;
        bool bLayoutInitialized;
        GHIUniformBufferLayout Layout;
        std::vector<FMember> Members;

        //void AddResourceTableEntriesRecursive(const char* UniformBufferName, const char* Prefix, uint16& ResourceIndex, TMap<FString, FResourceTableEntry>& ResourceTableMap) const;
    };

    struct IStructReflection
    {
        virtual void Start(const char* StructName) = 0;
        virtual void Add(const char* Type, const char* Name) = 0;
        virtual void End() = 0;
    };

    template <class T> const char* GetHLSL();

    template <> inline const char* GetHLSL<float>() { return "float"; }
    template <> inline const char* GetHLSL<uint32_t>() { return "uint"; }

    #define START_STRUCT(cbname)\
     struct cbname { static void Reflection(IStructReflection& r)\
     { r.Start(#cbname);

    #define ENTRY(type, name)\
     _Refl_##name(r); }\
     public:\
     type name;\
     private:\
     static void _Refl_##name(IStructReflection& r)\
     { r.Add(GetHLSL<type>(),#name);

    #define END_STRUCT() r.End(); } };

    struct SStructReflection : public IStructReflection
    {
        FILE* out;
        SStructReflection(FILE* InOut) : out(InOut) {}

        virtual void Start(const char* StructName)
        {
            fprintf(out, "\r\ncbuffer %s\r\n{\r\n", StructName);
        }
        virtual void Add(const char* Type, const char* Name)
        {
            fprintf(out, "\t%s %s;\r\n", Type, Name);
        }
        virtual void End()
        {
            fprintf(out, "};\r\n");
        }
    };
    void GenerateAutoCommon()
    {
        FILE* out = 0;
        if (_wfopen_s(&out, L"Shaders\\AutoCommon.hlsl", L"wb") == 0)
        {
            SStructReflection Refl(out);

            fclose(out);
        }
    }

}