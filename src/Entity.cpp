#include <iostream>
#include <iomanip>
#include "Entity.hpp"
#include "GraphicsManager.hpp"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"

Entity::Entity(ObjectBuffers *ob, ShaderProgram *sp, GraphicsManager *gr, double x, double y, double z, TextureType tex0)
    : objectBuffers(ob), shaderProgram(sp), grMan(gr), x(x), y(y), z(z), tex0(tex0), tex1(TextureType::None) {
        updateMatrixM();

    /*matrixM[0][0] =  0.588454;
    matrixM[0][1] = -0.00688001;
    matrixM[0][2] = -0.808502;

    matrixM[1][0] =  0.0680882;
    matrixM[1][1] =  0.997148;
    matrixM[1][2] =  0.0325452;

    matrixM[2][0] =  0.805972;
    matrixM[2][1] = -0.0742007;
    matrixM[2][2] =  0.587244;*/
}

Entity::~Entity() { }

void Entity::onLoop() {
    // miejsce na kod na przyszłość
}

void Entity::onRender(const Camera &c) {
    shaderProgram->use();

    glUniformMatrix4fv(shaderProgram->getUniformLocation("P"), 1, false, value_ptr(c.getMatrixP()));
    glUniformMatrix4fv(shaderProgram->getUniformLocation("V"), 1, false, value_ptr(c.getMatrixV()));
    glUniformMatrix4fv(shaderProgram->getUniformLocation("M"), 1, false, value_ptr(matrixM));

    glUniform4f(shaderProgram->getUniformLocation("lightPosition"), 0, 5, 5, 1);

    glUniform1i(shaderProgram->getUniformLocation("textureMap0"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, grMan->getTexture(tex0));

    if (tex1 != TextureType::None) {
        glUniform1i(shaderProgram->getUniformLocation("textureMap1"), 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, grMan->getTexture(tex1));
    }

    // Uaktywnienie VAO i tym samym uaktywnienie predefiniowanych w tym VAO powiązań slotów atrybutów z tablicami z danymi
    glBindVertexArray(objectBuffers->getVAO());

    // Narysowanie obiektu
    glDrawArrays(GL_TRIANGLES, 0, objectBuffers->getVertexCount());

    // Posprzątanie po sobie
    glBindVertexArray(0);
}

void Entity::move(double movX, double movY, double movZ) {
    x += movX;
    y += movY;
    z += movZ;
    updateMatrixM();
}

void Entity::setPosition(double x, double y, double z) {
    this->x = x;
    this->y = y;
    this->z = z;
    updateMatrixM();
}

void Entity::roll(double rotAngle) {
    angle += rotAngle;
    updateMatrixM();
}

void Entity::setAngle(double angle) {
    this->angle = angle;
    updateMatrixM();
}

void Entity::scale(double scale) {
    this->scaleCoeff = scale;
    updateMatrixM();
}

void Entity::setTexture(int num, TextureType tex) {
	if (num == 0)
		tex0 = tex;
	else
		tex1 = tex;
}

void Entity::setRotationM(CvMatr32f m) {
	matrixM = mat4(1.0f);

	// Transpozycja
	mat4 trans = matrixM;
	for (int f=0; f<3; f++) {
		for (int c=0; c<3; c++) {
			matrixM[f][c] = m[f*3 + c];	//transposed
		}
	}

	/* Bez transpozycji
	for (int f = 0; f < 3; ++f)
		for (int c = 0; c < 3; ++c)
			matrixM[f][c] = m[f*3 + c];*/

	/*matrixM[0][3] = 0.0;
	matrixM[1][3] = 0.0;
	matrixM[2][3] = 0.0;
	matrixM[3][0] = 0.0;
	matrixM[3][1] = 0.0;
	matrixM[3][2] = 0.0;
	matrixM[3][3] = 1.0;*/

	/*matrixM[0][0] = m[0];
    matrixM[0][1] = m[1];
    matrixM[0][2] = m[2];

    matrixM[1][0] = m[3];
    matrixM[1][1] = m[4];
    matrixM[1][2] = m[5];

    matrixM[2][0] = m[6];
    matrixM[2][1] = m[7];
    matrixM[2][2] = m[8];*/
}

void Entity::setTranslationM(CvMatr32f m) {
	matrixM = mat4(1.0f);

	matrixM[3][0] = -2*m[0] + 2.5;
	matrixM[3][1] = -2*m[1] + 2.5;
	matrixM[3][2] = -1.5*m[2] + 7.5;

	cout << fixed << setw(15) << "\rTx:" << m[0] << "\t\tTy:" << m[1] << "\t\tTz:" << m[2];
}

TextureType Entity::getTexture(int num) const {
	if (num == 0) return tex0;
	else		  return tex1;
}

void Entity::updateMatrixM() {
    matrixM = glm::scale(mat4(1.0f), vec3(scaleCoeff, scaleCoeff, scaleCoeff));
    matrixM = translate(matrixM, vec3(x, y, z));
    matrixM = rotate(matrixM, angle, vec3(0.5f, 1.0f, 0.0f));

    //cout << glm::to_string(matrixM) << "\n\n";
}
