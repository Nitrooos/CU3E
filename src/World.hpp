#ifndef WORLD
#define WORLD

#include <vector>
#include <list>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Image.hpp>

using namespace std;
using namespace sf;

#include "glm/glm.hpp"
#include "Entity.hpp"
#include "Camera.hpp"
#include "GraphicsManager.hpp"

class World {
    public:
        World(GraphicsManager *gm);
        virtual ~World();

        void onKeyboardEvent(Event e);
        void onMouseEvent(const Vector2<int> &pos);
        void onLoop();
        void onRender();
    private:
        // akt stworzenia
        void onInit();

        GraphicsManager *grMananger;            // manager obiektów rysowanych na scenie
        vector<Entity> objects;                 // obiekty należące do świata
        Camera camera{0.0, 0.0, 6.0};           // kamera, przez którą ogląda świat użytkownik
        bool rotateObjects{false};

        static constexpr float
                         sensitivity = 0.2f;    // jednostka zmiany położenia kamery
        float go{0.0},                          // o ile przesunąć kamerę w następnej iteracji onLoop (bliżej, dalej)
              side{0.0},                        // o ile przesunąć kamerę w następnej iteracji onLoop (w lewo, w prawo)
              height{0.0};                      // o ile przesunąć kamerę w następnej iteracji onLoop (wyżej, niżej)
};

#endif /* end of include guard: WORLD */
