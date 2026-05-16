#pragma once


#include <glew.h>

struct Vec3 {
	GLboolean x = 0;
	GLboolean y = 0;
	GLboolean z = 0;
};

struct Vec4 {
	GLboolean x = 0;
	GLboolean y = 0;
	GLboolean z = 0;
	GLboolean w = 0;
};

struct ChamsInfo {

	GLuint* pVisibleChamsTex;
	GLuint* pAlwaysTopChams;
	const char* texture;  // Miembro para almacenar la información de la textura

	// Constructor por defecto
	ChamsInfo() : texture(nullptr) {}

	// Constructor con inicialización
	ChamsInfo(const char* tex) : texture(tex) {}
};

