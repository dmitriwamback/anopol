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
private:
    Pipeline batchingPipeline, shadowPipeline, gBufferPipeline, instancePipeline;
};
}

#endif /* scene_h */
