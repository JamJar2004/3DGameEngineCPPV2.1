#include "AssimpMeshLoader.hpp"

#include "../Scene.hpp"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

static Model ProcessMesh(const aiMesh* mesh)
{
    std::vector<DefaultVertex> vertices;
    std::vector<uint32_t> indices;
    //std::vector<Texture> textures;

    for(size_t i = 0; i < mesh->mNumVertices; i++)
    {
        auto position = mesh->mVertices[i];

        auto texCoord = aiVector3D(0);

        if(mesh->mTextureCoords[0])
        {
            texCoord = mesh->mTextureCoords[0][i];
        }

        auto normal = aiVector3D(0);

        if(mesh->mNormals)
        {
            normal = mesh->mNormals[i];
        }

        auto tangent = aiVector3D(0);

        if(mesh->mTangents)
        {
            tangent = mesh->mTangents[i];
        }

        vertices.emplace_back(
            glm::vec3(position.x, position.y, position.z),
            glm::vec2(texCoord.x, texCoord.y),
            glm::vec3(normal.x, normal.y, normal.z),
            glm::vec3(tangent.x, tangent.y, tangent.z));
    }

    for(size_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(size_t j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    return Model(vertices, indices);
}

static void ProcessNode(std::vector<Model>& models, aiNode *node, const aiScene *scene)
{
    for(size_t i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        models.push_back(ProcessMesh(mesh));
    }

    for(size_t i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(models, node->mChildren[i], scene);
    }
}

MeshHandle AssimpMeshLoader::Load(Scene& scene, std::filesystem::path path) const
{
    uint32_t flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_FlipWindingOrder;
    if(SmoothNormals)
    {
        flags |= aiProcess_GenSmoothNormals;
    }
    else
    {
        flags |= aiProcess_GenNormals;
    }

    Assimp::Importer importer;
    const aiScene* assimpScene = importer.ReadFile(path.c_str(), flags);

    if(!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return nullptr;
    }

    std::vector<Model> models;
    ProcessNode(models, assimpScene->mRootNode, assimpScene);
    return scene.CreateMesh(models[0]);
}
