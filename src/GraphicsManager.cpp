#include "GraphicsManager.hpp"
#include "Tga.hpp"

#include <iostream>

GraphicsManager::GraphicsManager() { }

void GraphicsManager::onLoad() {
    shaders.emplace(ShaderType::Standard,
                    unique_ptr<ShaderProgram> (new ShaderProgram("src/shaders/vshader.glsl", nullptr, "src/shaders/fshader.glsl")));
    notifyObservers(33, false);

    objects.emplace(ObjectType::Cube,
                    unique_ptr<ObjectBuffers> (new ObjectBuffers(shaders[ShaderType::Standard], ObjectType::Cube)));
    /*objects.emplace(ObjectType::Teapot,
                    unique_ptr<ObjectBuffers> (new ObjectBuffers(shaders[ShaderType::Standard], "models/teapot.obj")));*/
    notifyObservers(33);

    textures.emplace(TextureType::Dice,  			readTextureFromFile("data/textures/dice.tga"));
    textures.emplace(TextureType::Crate,  			readTextureFromFile("data/textures/crate.tga"));
    textures.emplace(TextureType::Metal,  			readTextureFromFile("data/textures/metal.tga"));
    textures.emplace(TextureType::Grass,  			readTextureFromFile("data/textures/grass.tga"));
    textures.emplace(TextureType::Pumpkin,  		readTextureFromFile("data/textures/pumpkin.tga"));
    textures.emplace(TextureType::TNT,  			readTextureFromFile("data/textures/tnt.tga"));
    textures.emplace(TextureType::Brick,  			readTextureFromFile("data/textures/brick.tga"));
    textures.emplace(TextureType::Diamond,  		readTextureFromFile("data/textures/diamond.tga"));
    textures.emplace(TextureType::Diamond_block,  	readTextureFromFile("data/textures/diamond_block.tga"));
    textures.emplace(TextureType::Lava,  			readTextureFromFile("data/textures/lava.tga"));
    notifyObservers(34);
}

GraphicsManager::~GraphicsManager() {
    for (auto &x : textures) {
        glDeleteTextures(1, &x.second);
    }
}

ShaderProgram *GraphicsManager::getShader(ShaderType id) {
   return shaders[id].get();
}

ObjectBuffers *GraphicsManager::getBuffer(ObjectType id) {
   return objects[id].get();
}

GLuint GraphicsManager::getTexture(TextureType id) {
    return textures[id];
}

map<TextureType, GLuint> const& GraphicsManager::getTextures() const {
	return textures;
}

GLuint GraphicsManager::readTextureFromFile(const string &filename) {
    GLuint tex;
    TGAImg img;

    glActiveTexture(GL_TEXTURE0);
    if (img.Load(filename) == IMG_OK) {
        glGenTextures(1, &tex);                     // Zainicjuj uchwyt tex
        glBindTexture(GL_TEXTURE_2D, tex);          // Przetwarzaj uchwyt tex
        if (img.GetBPP() == 24)                     // Obrazek 24bit
            glTexImage2D(GL_TEXTURE_2D, 0, 3, img.GetWidth(), img.GetHeight(), 0, GL_RGB,GL_UNSIGNED_BYTE, img.GetImg());
        else if (img.GetBPP() == 32)                // Obrazek 32bit
                glTexImage2D(GL_TEXTURE_2D, 0, 4, img.GetWidth(), img.GetHeight(), 0, GL_RGBA,GL_UNSIGNED_BYTE, img.GetImg());
        else {
            // InvalidImageFormatException
        }
    } else {
        // LoadImageErrorException
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return tex;
}
