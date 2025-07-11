//
//  model.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-10-29.
//

#ifndef asset_h
#define asset_h

namespace anopol::render {

class Asset {
public:
    
    typedef struct Mesh {
        std::vector<Vertex>     vertices;
        std::vector<uint32_t>   indices;
        VertexBuffer vertexBuffer;
        IndexBuffer indexBuffer;
        Asset* parent;
    } Mesh;
    
    enum ModelType {
        OBJ
    };
    
    std::vector<Mesh> meshes;
    glm::vec3 position, rotation, scale;
    
    static Asset* Create(std::string assetPath);
    void PushInstance(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation, glm::vec3 color);
    void AllocInstances();
    bool IsInstanced();
    
    InstanceBuffer* GetInstances();
    
private:
    InstanceBuffer* instanceBuffer;
    void ProcessNode(aiNode *node, const aiScene *scene);
    void ProcessMesh(aiMesh *mesh, const aiScene *scene);
};

Asset* Asset::Create(std::string assetPath) {
    
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(assetPath.c_str(),
                                             aiProcess_Triangulate |
                                             aiProcess_FlipUVs |
                                             aiProcess_JoinIdenticalVertices |
                                             aiProcess_GenSmoothNormals | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph);
    
    Asset* asset = new Asset();
    
    asset->scale    = glm::vec3(1.0f, 1.0f, 1.0f);
    asset->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    asset->position = glm::vec3(0.0f);
    
    aiNode* rootNode = scene->mRootNode;
    asset->ProcessNode(rootNode, scene);
    
    return asset;
}


void Asset::ProcessNode(aiNode *node, const aiScene *scene) {
    
    for (int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(mesh, scene);
    }
    for (int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene);
    }
}
void Asset::ProcessMesh(aiMesh *mesh, const aiScene *scene) {
    
    std::vector<Vertex>     m_vertices;
    std::vector<uint32_t>   m_indices;
    
    Mesh m_mesh{};
    Vertex vertex;
    
    for (int i = 0; i < mesh->mNumVertices; i++) {
        glm::vec3 vertexVector;
        vertexVector.x = mesh->mVertices[i].x;
        vertexVector.y = mesh->mVertices[i].y;
        vertexVector.z = mesh->mVertices[i].z;
        
        vertex.vertex = vertexVector;
        
        glm::vec3 normalVector;
        normalVector.x = mesh->mNormals[i].x;
        normalVector.y = mesh->mNormals[i].y;
        normalVector.z = mesh->mNormals[i].z;
        
        vertex.normal = normalVector;
        
        if (mesh->mTextureCoords[0]) {
            glm::vec2 uv;
            uv.x = mesh->mTextureCoords[0][i].x;
            uv.y = mesh->mTextureCoords[0][i].y;
            vertex.uv = uv;
        }
        else {
            vertex.uv = glm::vec2(0.0f);
        }
                
        m_vertices.push_back(vertex);
    }
    
    for (int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (int j = 0; j < face.mNumIndices; j++) {
            m_indices.push_back(face.mIndices[j]);
        }
    }
    
    m_mesh.vertices = m_vertices;
    m_mesh.indices = m_indices;
    
    m_mesh.vertexBuffer.alloc(m_mesh.vertices);
    m_mesh.indexBuffer.alloc(m_mesh.indices);
    m_mesh.parent = this;
    
    meshes.push_back(m_mesh);
}

void Asset::PushInstance(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation, glm::vec3 color) {
    
    if (instanceBuffer == nullptr) {
        instanceBuffer = new InstanceBuffer();
        instanceBuffer->alloc(1000000);
    }
    instanceBuffer->appendInstance(position, scale, rotation, color);
}

void Asset::AllocInstances() {
    instanceBuffer->allocInstances();
}

bool Asset::IsInstanced() {
    return !(instanceBuffer == nullptr);
}

InstanceBuffer* Asset::GetInstances() {
    return instanceBuffer;
}


}



#endif /* model_h */
