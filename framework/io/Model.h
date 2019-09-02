//=================================================================================================
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "PCH.h"

#include "mymath.h"
#include "Serialization.h"

#include "GHIResources.h"
#include "GHICommandContext.h"

// Assimp
#include "Importer.hpp"
#include "scene.h"
#include "postprocess.h"

struct aiMesh;

namespace GHI
{

class SDKMesh;


struct MaterialMap
{
    std::string DiffuseMapName;
    std::string NormalMapName;
    std::string RoughnessMapName;
    std::string MetallicMapName;

    GHITexture *DiffuseMap;
    GHITexture *NormalMap;
    GHITexture *RoughnessMap;
    GHITexture *MetallicMap;

    std::string str() const
    {
        char buf[1024];
        snprintf(buf, 1024, "Mesh Material:"
            "\n\t\t diffuse-map [%s]"
            "\n\t\t normal-map [%s]"
            "\n\t\t roughness-map [%s]"
            "\n\t\t metallic-map  [%s]", 
            DiffuseMapName.c_str(),
            NormalMapName.c_str(),
            RoughnessMapName.c_str(),
            MetallicMapName.c_str());
        return std::string(buf);
    }

    template<typename TSerializer> void Serialize(TSerializer& serializer)
    {
        SerializeItem(serializer, DiffuseMapName);
        SerializeItem(serializer, NormalMapName);
        SerializeItem(serializer, RoughnessMapName);
        SerializeItem(serializer, MetallicMapName);
    }
};

struct MeshPart
{
    uint32_t VertexStart;
    uint32_t VertexCount;
    uint32_t IndexStart;
    uint32_t IndexCount;
    uint32_t MaterialIdx;

    MeshPart() : VertexStart(0), VertexCount(0), IndexStart(0), IndexCount(0), MaterialIdx(0)
    { }
};

class Mesh
{
    friend class Model;

public:

    // Init from loaded files
    void InitFromSDKMesh(IGHIComputeCommandCotext* commandcontext, SDKMesh& sdkmesh, uint32 meshIdx, bool generateTangents);
    void InitFromAssimpMesh(IGHIComputeCommandCotext* commandcontext, const aiMesh& assimpMesh);

    // Procedural generation
    void InitBox(IGHIComputeCommandCotext* commandcontext, const Float3& dimensions, const Float3& position,
                 const Quaternion& orientation, uint32 materialIdx);

    void InitPlane(IGHIComputeCommandCotext* commandcontext, const Float2& dimensions, const Float3& position,
                   const Quaternion& orientation, uint32 materialIdx);

    void InitCornea(IGHIComputeCommandCotext* commandcontext, uint32 materialIdx);

    std::vector<MeshPart>& MeshParts()
    {
        return meshParts;
    }
    const std::vector<MeshPart>& MeshParts() const
    {
        return meshParts;
    }

    const std::vector<GHIInputElementInfo>& VertexElements() const
    {
        return inputElements;
    }

    uint32 VertexStride() const { return vertexStride; }
    uint32 NumVertices() const { return numVertices; }
    uint32 NumIndices() const { return numIndices; }

    GHIIndexType IndexBufferType() const
    {
        return indexType;
    }

    uint32 IndexSize() const
    {
        return indexType == GHIIndexType::Index32Bit ? 4 : 2;
    }

    const uint8* Vertices() const { return vertices.data(); }
    const uint8* Indices() const { return indices.data(); }

    GHIBuffer* VertexBuffer() const { return vertexBuffer; }
    GHIBuffer* IndexBuffer() const { return indexBuffer; }

    template<typename TSerializer> void Serialize(TSerializer& serializer)
    {
#if 0
        SerializeRawVector(serializer, meshParts);

        inputElementStrings.resize(inputElements.size());
        for(uint64 i = 0; i < inputElements.size(); ++i)
            inputElementStrings[i] = std::string(inputElements[i].SemanticName);

        SerializeRawVector(serializer, inputElements);
        SerializeItem(serializer, inputElementStrings);

        for(uint64 i = 0; i < inputElements.size(); ++i)
            inputElements[i].SemanticName = inputElementStrings[i].c_str();

        SerializeItem(serializer, vertexStride);
        SerializeItem(serializer, numVertices);
        SerializeItem(serializer, numIndices);
        uint32 idxType = uint32(indexType);
        SerializeItem(serializer, idxType);
        indexType = IndexType(idxType);
        SerializeRawVector(serializer, vertices);
        SerializeRawVector(serializer, indices);
#endif
    }

    std::string str() const
    {
        char buf[1024];
        snprintf(buf, 1024, "Mesh :"
            "\n\t\t Vertex count: %d"
            "\n\t\t Index count: %d"
            "\n\t\t Tri Face count: %d"
            "\n\t\t Index type: %s"
            "\n\t\t Vertext attributes: %s"
            "\n\t\t Vertex stream count: 1\n",
            numVertices,
            numIndices,
            numTriFaces,
            IndexTypeStr().c_str(),
            VertexAttributeStr().c_str());

        return std::string(buf);
    }

    // Rendering
    void Render(IGHIComputeCommandCotext* commandcontext) const;

	AABB boundingbox;
protected:

    void GenerateTangentFrame();
    //void CreateInputElements(const D3DVERTEXELEMENT9* declaration);
    void CreateVertexAndIndexBuffers(IGHIComputeCommandCotext* commandcontext);

    GHIBuffer *vertexBuffer = nullptr;
    GHIBuffer *indexBuffer = nullptr;

    std::vector<MeshPart> meshParts;
    std::vector<GHIInputElementInfo> inputElements;

    int vertexStride = 0;
    uint32 numVertices = 0;
    uint32 numIndices = 0;
    uint32 numTriFaces = 0;

    GHIIndexType indexType = GHIIndexType::Index16Bit;

    std::vector<uint8> vertices;
    std::vector<uint8> indices;

    std::string IndexTypeStr() const
    {
        return indexType == GHIIndexType::Index16Bit ? "16-bit" : "32-bit";
    }

    std::string VertexAttributeStr() const
    {
        std::string ret;
        for (uint64 i = 0; i < inputElements.size(); ++i)
             ret += std::string(inputElements[i].SemanticName)+"\t";
        return ret;
    }
};

class Model
{
public:

    // Loading from file formats
    void CreateFromSDKMeshFile(IGHIComputeCommandCotext* commandcontext,
                                const char* fileName,
                                const char* normalMapSuffix = NULL,
                                bool generateTangentFrame = false,
                                bool overrideNormalMaps = false,
                                bool forceSRGB = false);

    void CreateWithAssimp(IGHIComputeCommandCotext* commandcontext, const char* fileName, bool forceSRGB = false);

    void CreateFromMeshData(IGHIComputeCommandCotext* commandcontext, const char* fileName, bool forceSRGB = false);

    // Procedural generation
    void GenerateBoxScene(IGHIComputeCommandCotext* commandcontext,
                          const Float3& dimensions = Float3(1.0f, 1.0f, 1.0f),
                          const Float3& position = Float3(),
                          const Quaternion& orientation = Quaternion(),
                          const char* colorMap = "",
                          const char* normalMap = "");
    void GenerateBoxTestScene(IGHIComputeCommandCotext* commandcontext);
    void GeneratePlaneScene(IGHIComputeCommandCotext* commandcontext,
                            const Float2& dimensions = Float2(1.0f, 1.0f),
                            const Float3& position = Float3(),
                            const Quaternion& orientation = Quaternion(),
                            const char* colorMap = "",
                            const char* normalMap = "");

    void GenerateCorneaScene(IGHIComputeCommandCotext* commandcontext);

    // Accessors
    std::vector<MaterialMap>& Materials() { return meshMaterials; };
    const std::vector<MaterialMap>& Materials() const { return meshMaterials; };

    std::vector<Mesh>& Meshes() { return meshes; }
    const std::vector<Mesh>& Meshes() const { return meshes; }

    // Serialization
    template<typename TSerializer>
    void Serialize(TSerializer& serializer, IGHIComputeCommandCotext* commandcontext, bool forceSRGB = false)
    {
#if 0
        SerializeItem(serializer, meshes);
        SerializeItem(serializer, meshMaterials);
        SerializeItem(serializer, fileDirectory);

        if(TSerializer::IsReadSerializer())
        {
            for(uint64 i = 0; i  < meshes.size(); ++i)
                meshes[i].CreateVertexAndIndexBuffers(device);

            for(uint64 i = 0; i < meshMaterials.size(); ++i)
                LoadMaterialResources(meshMaterials[i], fileDirectory, device, forceSRGB);
        }
#endif
    }

protected:

    static void LoadMaterialResources(MaterialMap& material, const std::string& directory, IGHIComputeCommandCotext* commandcontext, bool forceSRGB);

    std::vector<Mesh> meshes;
    std::vector<MaterialMap> meshMaterials;
    std::string fileDirectory;
};

}