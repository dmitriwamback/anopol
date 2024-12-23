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
    glm::mat4 model;
};

}

#endif /* push_constants_h */