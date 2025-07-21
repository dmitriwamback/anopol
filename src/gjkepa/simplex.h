//
//  simplex.h
//  anopol
//
//  Created by Dmitri Wamback on 2025-04-13.
//

#ifndef simplex_h
#define simplex_h

namespace anopol::collision {
class Simplex {
private:
    std::array<glm::vec3, 4> points;
    int simplexSize;
    
public:
    Simplex() : simplexSize(0) {}
    
    Simplex& operator=(std::initializer_list<glm::vec3> list) {
        for (auto v = list.begin(); v != list.end(); v++) {
            points[std::distance(list.begin(), v)] = *v;
        }
        simplexSize = list.size();
        return *this;
    }
    glm::vec3& operator[](unsigned i) { return points[i]; }
    
    void pushFront(glm::vec3 point) {
        if (simplexSize < 4) {
            for (int i = simplexSize; i > 0; i--) {
                points[i] = points[i - 1];
            }
            points[0] = point;
            simplexSize++;
        }
        else {
            for (int i = 3; i > 0; i--) {
                points[i] = points[i - 1];
            }
            points[0] = point;
        }
    }
    unsigned size() const { return simplexSize; }
    auto begin() const { return points.begin(); }
    auto end() const { return points.end() - (4 - simplexSize); }
};
}


#endif /* simplex_h */
