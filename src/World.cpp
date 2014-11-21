#include "World.hpp"
#include "App.hpp"

#include <iostream>

World::World(GraphicsManager *gm) : grMananger(gm) {
    objects.push_back(Entity{grMananger->getBuffer(ObjectType::Cube),
                             grMananger->getShader(ShaderType::Standard),
                             0, 0, 0,
                             grMananger->getTexture(TextureType::Brick)}
                     );
    onInit();
}

World::~World() { }

void World::onKeyboardEvent(Event e) {
    // zakładamy, że obiekty mogą reagować tylko na wciśnięcie klawisza
    switch (e.type) {
        case Event::KeyPressed:
            switch (e.key.code) {
                case Keyboard::W: go     = -sensitivity; break;
                case Keyboard::S: go     =  sensitivity; break;
                case Keyboard::A: side   = -sensitivity; break;
                case Keyboard::D: side   =  sensitivity; break;
                case Keyboard::K: height =  sensitivity; break;
                case Keyboard::M: height = -sensitivity; break;
                case Keyboard::X: rotateObjects = !rotateObjects; break;
            }
            break;
        case Event::KeyReleased:
            switch (e.key.code) {
                case Keyboard::W: case Keyboard::S: go     = 0.0f; break;
                case Keyboard::A: case Keyboard::D: side   = 0.0f; break;
                case Keyboard::K: case Keyboard::M: height = 0.0f; break;
            }
            break;
    }
}

void World::onMouseEvent(const Vector2<int> &pos) {
    int diffX = App::getWindowCenterX() - pos.x,
        diffY = App::getWindowCenterY() - pos.y;

    camera.roll(diffX/400.0f, diffY/400.0f);
}

void World::onLoop() {
    if (this->rotateObjects) {
        float angle = 1.0f;
        for (auto &x : objects) {
            x.roll(angle);
            angle *= -1;
        }
    }

    float xShift = -camera.getZShift(this->go) + camera.getXShift(this->side),
          yShift =  this->height,
          zShift =  camera.getXShift(this->go) + camera.getZShift(this->side);

    static float arg = 0.0f;
    float scale = 1.5*abs(sin(arg)) + 0.5;
    objects.back().scale(scale);
    arg += 0.01f;

    if (xShift || yShift || zShift)
        camera.movEye(xShift, yShift, zShift);

    //camera.writeCoordinates();
}

void World::onRender() {
    // renderuj każdy obiekt świata po kolei
    for (auto &x : objects)
        x.onRender(camera);
}

void World::onInit() {
    glEnable(GL_DEPTH_TEST);
}
