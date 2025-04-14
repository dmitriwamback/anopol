//
//  mesh_combine_structs.h
//  anopol
//
//  Created by Dmitri Wamback on 2025-04-13.
//

#ifndef mesh_combine_structs_h
#define mesh_combine_structs_h

namespace anopol::batch {

typedef struct batchDrawInformation {
    uint32_t first;
    uint32_t indexCount;
    uint32_t vertexOffset;
    uint32_t object;
} batchDrawInformation;

}

#endif /* mesh_combine_structs_h */
