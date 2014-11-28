#ifndef ENTITY
#define ENTITY

#include "glm/glm.hpp"
#include <string>

using namespace std;
using namespace glm;

#include "Types.hpp"
#include "Camera.hpp"
#include "GraphicsManager.hpp"
#include "ShaderProgram.hpp"
#include "ObjectBuffers.hpp"

#include "opencv2/legacy/compat.hpp"

class Entity {
    public:
        Entity(ObjectBuffers *ob, ShaderProgram *sp, GraphicsManager *gr, double x, double y, double z, TextureType tex0);
        virtual ~Entity();

        void onLoop();
        void onRender(const Camera &c);

        void move(double movX, double movY, double movZ);
        void setPosition(double x, double y, double z);
        void roll(double rotAngle);
        void setAngle(double angle);
        void scale(double scale);
        void setTexture(int num, TextureType tex);
        void setRotationM(CvMatr32f m);
        void setTranslationM(CvMatr32f m);
        TextureType getTexture(int num) const;

        float getX() const { return x; }
        float getY() const { return y; }
        float getZ() const { return z; }
    private:
        void updateMatrixM();

        ShaderProgram *shaderProgram;           // pointer do programu cieniującego
        ObjectBuffers *objectBuffers;           // pointer do buforów vbo
        GraphicsManager *grMan;
        TextureType tex0, tex1;		            // tekstury obiektu (można użyć max dwóch)

        float   x, y, z,                        // wsp obiektu na scenie
                angle{0.0},                     // kąt obrotu (początkowo 0.0)
                scaleCoeff{1.0};

        mat4 matrixM;                           // macierz modelu
};

#endif /* end of include guard: Entity */
