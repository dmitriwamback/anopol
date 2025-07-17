//
//  scene.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-10-14.
//

#ifndef scene_h
#define scene_h

namespace anopol::pipeline {

class Scene {
public:
    static Scene Create();
    void RenderScene();
private:
    Pipeline batchingPipeline, shadowPipeline, gBufferPipeline, instancePipeline;
    Pipeline debugPipeline;
};

Scene Scene::Create() {
    Scene scene = Scene();
    
    scene.debugPipeline = Pipeline::CreatePipeline("/Users/dmitriwamback/Documents/Projects/anopol/anopol/shaders/main");
    
    
    
    return scene;
}

}

#endif /* scene_h */
