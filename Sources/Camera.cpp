#include "../Libraries/Camera.h"

Camera::Camera(glm::vec3 target, glm::vec3 pos) {
		position = pos;
		view = glm::mat4(1.0);
		front = glm::normalize(target - position);
		left = glm::cross(up, front);
		left.y = 0; left = glm::normalize(left);
		up = glm::cross(front, left);
		view = glm::lookAt(position, position + front, up);
}

Camera::Camera(glm::vec3 pos, GLfloat yaw, GLfloat pitch) {
	position = pos;
	view = glm::mat4(1.0);
	glm::vec3 direction = glm::vec3(sin(glm::radians(yaw)) * cos(glm::radians(pitch)),  sin(glm::radians(pitch)), cos(glm::radians(yaw)) * cos(glm::radians(pitch)));
	front = glm::normalize(direction);
	left = glm::cross(up, front);
	left.y = 0; left = glm::normalize(left);
	up = glm::cross(front, left);
	view = glm::lookAt(position, position + front, up);
}
