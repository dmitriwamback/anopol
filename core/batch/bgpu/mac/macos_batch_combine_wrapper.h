//
//  macos_batch_combine_wrapper.h
//  anopol
//
//  Created by Dmitri Wamback on 2025-05-01.
//

#ifndef macos_batch_combine_wrapper_h
#define macos_batch_combine_wrapper_h

namespace anopol::metal {

class BatchCombineImplementation;

class BatchCombine {
public:
    BatchCombine();
    ~BatchCombine();
    
    void Combine(std::vector<anopol::render::Vertex> vertices);
private:
    BatchCombineImplementation* implementation;
};

}

#endif /* macos_batch_combine_wrapper_h */
