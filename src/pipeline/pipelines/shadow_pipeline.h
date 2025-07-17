//
//  shadow_pipeline.h
//  anopol
//
//  Created by Dmitri Wamback on 2025-04-21.
//

#ifndef shadow_pipeline_h
#define shadow_pipeline_h

namespace anopol::pipeline {

class ShadowPipeline {
    
    anopol::structs::shadow shadowPipelines;
    anopol::structs::shadowImage shadowDepthImage;
    anopol::structs::shadowPushConstants shadowPushConstantsBlock;
    
    std::array<anopol::structs::shadowCascade, anopol_max_cascades> cascades;
    
};

}

#endif /* shadow_pipeline_h */
