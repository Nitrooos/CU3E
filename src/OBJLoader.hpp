#ifndef LOADEROBJ_H
#define LOADEROBJ_H

#include <string>

using namespace std;

struct OBJLoader {
    public:
        OBJLoader(string source);
        OBJLoader(float *vertices, float *texCoords, float *normals,
                  int vertexCount);
        ~OBJLoader();

        void write() const;

        float *vertices, *texture, *normals;
        int vertexCount;
    private:
        void load();

        int verticesCount, normalsCount, coordsCount, facesCount;
        string source;
};

#endif
