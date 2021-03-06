#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include "GL/glew.h"
#include "stdio.h"
#include <string>

using namespace std;

class ShaderProgram {
    private:
        GLuint shaderProgram; //Uchwyt reprezentujący program cieniujacy
        GLuint vertexShader; //Uchwyt reprezentujący vertex shader
        GLuint geometryShader; //Uchwyt reprezentujący geometry shader
        GLuint fragmentShader; //Uchwyt reprezentujący fragment shader
        char* readFile(char* fileName); //metoda wczytująca plik tekstowy do tablicy znaków
        GLuint loadShader(GLenum shaderType,char* fileName); //Metoda wczytuje i kompiluje shader, a następnie zwraca jego uchwyt
    public:
        ShaderProgram(char* vertexShaderFile,char* geometryShaderFile,char* fragmentShaderFile);
        ~ShaderProgram();
        void use() const; //Włącza wykorzystywanie programu cieniującego
        GLuint getUniformLocation(const string &variableName) const;        //Pobiera numer slotu związanego z daną zmienną jednorodną
        GLuint getAttribLocation(const string &variableName) const;         //Pobiera numer slotu związanego z danym atrybutem
};



#endif
