//
//  model.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-10-29.
//

#ifndef asset_h
#define asset_h

namespace anopol::render {

class Asset: public Renderable {
public:
    Renderable* Create(std::string assetPath);
};
}



#endif /* model_h */
