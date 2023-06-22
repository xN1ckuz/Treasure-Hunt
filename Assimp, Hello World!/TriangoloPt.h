#pragma once

using namespace std;

class TriangoloPt {
public:
    vector<glm::vec3> listaVertici;
    float x, y, z;

    static constexpr int DIM = 3;

    TriangoloPt(glm::vec3 pos) {
        listaVertici = {};
        x = pos.x;
        y = pos.y;
        z = pos.z;
    }

    TriangoloPt(glm::vec3 a, glm::vec3 b, glm::vec3 c){
        listaVertici.push_back(a);
        listaVertici.push_back(b);
        listaVertici.push_back(c);
        glm::vec3 baricentro = calcolaBaricentro();
        x = baricentro.x;
        y = baricentro.y;
        z = baricentro.z;
    }

    glm::vec3 calcolaBaricentro() {
        if (listaVertici.size() == 0) {
            return glm::vec3(0.0, 0.0, 0.0);
        }
        float x = (listaVertici[0].x + listaVertici[1].x + listaVertici[2].x) / 3;
        float y = (listaVertici[0].y + listaVertici[1].y + listaVertici[2].y) / 3;
        float z = (listaVertici[0].z + listaVertici[1].z + listaVertici[2].z) / 3;
        return(glm::vec3(x, y, z));
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

    bool operator==(const TriangoloPt& p) const {
        return x == p.x && y == p.y && z == p.z;
    }

    glm::vec3 toGLM() {
        return glm::vec3(x, y, z);
    }
};
