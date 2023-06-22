#pragma once

class Punto3D {
public:
    float x, y, z;

    static constexpr int DIM = 3;
    
    Punto3D() {
        x = 0.0;
        y = 0.0;
        z = 0.0;
    }

    Punto3D (glm::vec3 v){
        x = v.x;
        y = v.y;
        z = v.z;
    }

    float& operator[](int index) {
        switch (index) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        default:
            throw std::out_of_range("indice fuori range");
        }
    }

    const float& operator[](int index) const {
        switch (index) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        default:
            throw std::out_of_range("indice fuori range");
        }
    }

    bool operator==(const Punto3D& p) const {
        return x == p.x && y == p.y && z == p.z;
    }

    glm::vec3 toGLM() {
        return glm::vec3(x, y, z);
    }
};
