//
//  mesh_combine.h
//  anopol
//
//  Created by Dmitri Wamback on 2025-04-13.
//

#ifndef mesh_combine_h
#define mesh_combine_h

namespace anopol::batch {

class MeshCombineGroup {
public:
    std::vector<anopol::render::Renderable*> renderables;
    std::vector<anopol::render::Asset*> assets;
    
    static MeshCombineGroup Create();
    void Append(anopol::render::Renderable* renderable);
    void Append(anopol::render::Asset* asset);
    void Append(std::vector<anopol::render::Renderable*>& renderables);
    void Append(std::vector<anopol::render::Asset*>& assets);
    void Reserve(uint32_t renderableCount, uint32_t assetCount);
};

MeshCombineGroup MeshCombineGroup::Create() {
    return MeshCombineGroup();
}

void MeshCombineGroup::Append(anopol::render::Renderable* renderable) {
    renderables.push_back(renderable);
}

void MeshCombineGroup::Append(anopol::render::Asset* asset) {
    assets.push_back(asset);
}

void MeshCombineGroup::Append(std::vector<anopol::render::Renderable*>& renderables) {
    this->renderables.reserve(this->renderables.size() + renderables.size());
    this->renderables.insert(this->renderables.end(), renderables.begin(), renderables.end());
}

void MeshCombineGroup::Append(std::vector<anopol::render::Asset*>& assets) {
    for (anopol::render::Asset* asset : assets) {
        this->assets.push_back(asset);
    }
}

void MeshCombineGroup::Reserve(uint32_t renderableCount, uint32_t assetCount) {
    renderables.reserve(renderableCount);
    assets.reserve(assetCount);
}

}


#endif /* mesh_combine_h */
