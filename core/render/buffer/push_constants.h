//
//  push_constants.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-12-22.
//

#ifndef push_constants_h
#define push_constants_h

namespace anopol::render {

struct anopolStandardPushConstants {
    glm::vec4 scale;
    glm::vec4 position;
    glm::vec4 rotation;
    glm::vec4 color;
    glm::mat4 model;
    int instanced = false;
    int batched = false;
    uint32_t padding = 0;
};

}

#endif /* push_constants_h */
