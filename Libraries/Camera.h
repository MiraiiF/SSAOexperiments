#pragma once
#include "General.h"
class Camera
{	
public:
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), front = glm::vec3(0.0f, 0.0f, -1.0f), up = glm::vec3(0.0f, 1.0f, 0.0f), left = glm::vec3(-1.0f, 0.0f, 0.0f);
public:
	glm::mat4 view = glm::mat4(1.0);
	Camera(glm::vec3 target, glm::vec3 pos);
	Camera(glm::vec3 pos, GLfloat yawex, GLfloat pitchex);
};

