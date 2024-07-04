#include "../Libraries/Shader.h"

Shader::Shader(const char* vertexsource, const char* fragmentsource)
{
	std::string vertexcode;
	std::string fragmentcode;
	std::ifstream vfile;
	std::ifstream ffile;
	vfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	ffile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		vfile.open(vertexsource);
		ffile.open(fragmentsource);
		std::stringstream vertexstream, fragmentstream;

		vertexstream << vfile.rdbuf();
		fragmentstream << ffile.rdbuf();

		vfile.close();
		ffile.close();

		vertexcode = vertexstream.str();
		fragmentcode = fragmentstream.str();
	}
	catch (std::ifstream::failure e) {
		std::cout << "FALHA NOS SHADERS - ARQUIVO NAO LIDO CORRETAMENTE - " << e.what() << "\n";
	}
	const char* vertexshadersource = vertexcode.c_str();
	const char* fragmentshadersource = fragmentcode.c_str();

	GLuint vertexshader, fragmentshader;
	GLint sucess;
	GLchar log[512];

	vertexshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexshader, 1, &vertexshadersource, NULL);
	glCompileShader(vertexshader);
	glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &sucess);
	if (!sucess) {
		glGetShaderInfoLog(vertexshader, 512, NULL, log);
		std::cout << "vertex falhou - " << log << std::endl;
	}
	fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentshader, 1, &fragmentshadersource, NULL);
	glCompileShader(fragmentshader);
	glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &sucess);
	if (!sucess) {
		glGetShaderInfoLog(fragmentshader, 512, NULL, log);
		std::cout << "fragment falhou - " << log << std::endl;
	}
	ID = glCreateProgram();
	glAttachShader(ID, vertexshader);
	glAttachShader(ID, fragmentshader);
	glLinkProgram(ID);
	glGetProgramiv(ID, GL_LINK_STATUS, &sucess);
	if (!sucess) {
		glGetProgramInfoLog(ID, 512, NULL, log);
		std::cout << "programa falhou - " << log << std::endl;
	}
	glDeleteShader(vertexshader);
	glDeleteShader(fragmentshader);
}

Shader::Shader(const char* computesource, unsigned int binding, glm::uvec2 size)
{
    worksize = size;
    std::string computecode;
    std::ifstream cfile;
    cfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        cfile.open(computesource);
        std::stringstream computestream;

        computestream << cfile.rdbuf();
        cfile.close();

        computecode = computestream.str();
    }
    catch (std::ifstream::failure e) {
        std::cout << "FALHA NOS SHADERS - ARQUIVO NAO LIDO CORRETAMENTE - " << e.what() << "\n";
    }

    const char* computeshadersource = computecode.c_str();

    GLuint computeshader;
    GLint sucess;
    GLchar log[512];

    computeshader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeshader, 1, &computeshadersource, NULL);
    glCompileShader(computeshader);
    glGetShaderiv(computeshader, GL_COMPILE_STATUS, &sucess);
    if (!sucess) {
        glGetShaderInfoLog(computeshader, 512, NULL, log);
        std::cout << "vertex falhou - " << log << std::endl;
    }
    ID = glCreateProgram();
    glAttachShader(ID, computeshader);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &sucess);
    if (!sucess) {
        glGetProgramInfoLog(ID, 512, NULL, log);
        std::cout << "programa falhou - " << log << std::endl;
    }
    glDeleteShader(computeshader);

    glGenTextures(1, &TexID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TexID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, size.x, size.y, 0, GL_RED, GL_FLOAT, NULL);
    glBindImageTexture(binding, TexID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
}

void Shader::use()
{
	glUseProgram(ID);
}

void Shader::setbool(const std::string& name, bool valor) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)valor);
}

void Shader::setint(const std::string& name, int valor) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), valor);
}

void Shader::setfloat(const std::string& name, float valor) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), valor);
}

void Shader::setfloat4(const std::string& name, float x, float y, float z, float w)
{
	glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

void Shader::setfloat3(const std::string& name, float x, float y, float z)
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::setmat4(const std::string& name, glm::mat4 trans)
{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &trans[0][0]);
}

void Shader::nope()
{
	glUseProgram(0);
}

void Shader::erase()
{
	glDeleteProgram(ID);
}

void Shader::compuse(){
    glUseProgram(ID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TexID);
}

void Shader::dispatch(){
    glDispatchCompute(100, 100 , 1);
}

void Shader::wait(){
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void Shader::setcomp(float* values){
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, worksize.x, worksize.y, 0, GL_RED, GL_FLOAT, values);
}

std::vector<float> Shader::getcomp(){
    std::vector<float> result(worksize.x*worksize.y);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, result.data());
    return result;
}

void Shader::setvec4(const std::string& name, glm::vec4 vector){
	setfloat4(name, vector.x, vector.y, vector.z, vector.w);
}

void Shader::setvec3(const std::string& name, glm::vec3 vector){
	setfloat3(name, vector.x, vector.y, vector.z);
}
