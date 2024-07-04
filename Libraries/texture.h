#pragma once
#include "General.h"

GLenum const ch[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
GLenum const sch[] = {GL_RED, GL_RG, GL_SRGB, GL_SRGB_ALPHA};

class texture
{
public:
	GLuint ID;
	int height, width, channels;

	texture(const char* path, GLint wrapmode, GLint filter, bool srgb);

	void bind(int location);

	void erase();
};

