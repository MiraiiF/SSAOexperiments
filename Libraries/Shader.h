#pragma once
#include "General.h"


class Shader
{
public:
	GLuint ID;
    GLuint TexID;
    glm::uvec2 worksize;

	Shader(const char* vertexsource, const char* fragmentsource);
    Shader(const char* computesource, unsigned int binding, glm::uvec2 size);

	void use();
    void compuse();
    void dispatch();
    void wait();
    void setcomp(float* values);
    std::vector<float> getcomp();

	void setbool(const std::string& name, bool valor) const;
	void setint(const std::string& name, int valor) const;
	void setfloat(const std::string& name, float valor) const;
	void setfloat4(const std::string& name, float x, float y, float z, float w);
	void setfloat3(const std::string& name, float x, float y, float z);
	void setmat4(const std::string& name, glm::mat4 trans);
	void setvec4(const std::string& name, glm::vec4 vector);
	void setvec3(const std::string& name, glm::vec3 vector);

	void nope();
	void erase();
};

