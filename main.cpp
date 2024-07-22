#include "Libraries/Camera.h"
#include "Libraries/texture.h"
#include "Libraries/Model.h"

bool firstcursor = true;
float lastx = 640, lasty = 360;
GLfloat yaw = 180, pitch = 0;
int largura = 1280, altura = 720;


void callback_redimensionamento(GLFWwindow* window, int width, int height);
void callback_cursor(GLFWwindow* janela, double xpos, double ypos);
void callback_CloseWindow(GLFWwindow* janela);
void constrain(float* value, float lim_up, float lim_down);
void up_down(GLFWwindow* janela, float* var, float val, int key_up, int key_down, float lim_up, float lim_down);
void camera_movement(GLFWwindow* janela, float velocity, Camera viewer, glm::vec3 *position);

int main(void){

    if (!glfwInit()) {
		printf("GLFW nono\n");
		return -1;
	}

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* janela = glfwCreateWindow(largura, altura, "Cabeludo", NULL, NULL);
	if (!janela) {
		printf("janela nao foi criada...\n");
		glfwTerminate();
		return -1;
	}

    glfwMakeContextCurrent(janela);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("no glad...");
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, largura, altura);
    glClearColor(0.05, 0.0, 0.3, 1.0);
	glfwSetFramebufferSizeCallback(janela, callback_redimensionamento); 
	glfwSetCursorPosCallback(janela, callback_cursor);
	glfwSetInputMode(janela, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	Shader Modellight("../Shaders/AssimpExample.vert", "../Shaders/AssimpExample.frag");
	Shader Shadow("../Shaders/Shadow.vert", "../Shaders/Shadow.frag");

	glEnable(GL_DEPTH_TEST);

	float angle = 0.0f;
	float angle_var = 0.0f;

	double time = 0;
	
	//Shadow Map
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
             SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);  

	float near_plane = 0.1f, far_plane = 50.0f;
	glm::mat4 lightProjection = glm::ortho(-35.0f, 35.0f, -35.0f, 35.0f, near_plane, far_plane);

	Camera dir_light(glm::vec3( -0.2f, 1.0f, -0.3f) + glm::vec3(-2.0f, 4.0f, -1.0f), 
					glm::vec3(-2.0f, 4.0f, -1.0f));
	
	glm::mat4 lightView = dir_light.view;

	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	Model parede("../models/parede/parede.obj");
	Model lixo("../models/lixo/lixo.obj");
	Model piso("../models/piso/piso.obj");

	glm::vec3 pos = glm::vec3(0.0f, 0.0f, 2.0f);
	glm::vec4 color;

	while (!glfwWindowShouldClose(janela))
	{
		
		//Render Scene from Light Pespective
		Shadow.use();
		Shadow.setmat4("lightSpaceMatrix", lightSpaceMatrix);
		
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    	glClear(GL_DEPTH_BUFFER_BIT);
		
		glm::mat4 model = glm::mat4(1.0f);

		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -3.0f));
		model = glm::scale(model, glm::vec3(1.0, 1.0, 1.0));

		Shadow.setmat4("model", model);
		parede.Draw(Shadow);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.5f));
		model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));

		Shadow.setmat4("model", model);
		lixo.Draw(Shadow);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.5f));

		Shadow.setmat4("model", model);
		piso.Draw(Shadow);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, largura, altura);

		callback_CloseWindow(janela);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
		glm::mat4 projection = glm::mat4(1.0f);

		projection = glm::perspective(glm::radians(45.0f), (GLfloat)(largura)/(GLfloat)(altura), 0.01f, 50.0f);
		
		Camera viewer = Camera(pos, yaw, pitch);

		float velocity = 5*(glfwGetTime() - time);
		time = glfwGetTime();
		camera_movement(janela, velocity, viewer, &pos);

		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -3.0f));
		model = glm::scale(model, glm::vec3(1.0, 1.0, 1.0));

		Modellight.use();
		Modellight.setmat4("projection", projection);
		Modellight.setmat4("view", viewer.view);
		Modellight.setmat4("model", model);
		Modellight.setvec3("light.direction", glm::vec3(-0.2f, 1.0f, -0.3f));
		Modellight.setvec3("light.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        Modellight.setvec3("light.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
        Modellight.setvec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		Modellight.setvec3("viewPos", viewer.position);
		Modellight.setfloat("material.shininess", 32);
		Modellight.setmat4("lightSpaceMatrix", lightSpaceMatrix);

		glBindTexture(GL_TEXTURE_2D, depthMap);

		parede.Draw(Modellight);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.5f));
		model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
		Modellight.setmat4("model", model);

		glBindTexture(GL_TEXTURE_2D, depthMap);

		lixo.Draw(Modellight);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.5f));
		Modellight.setmat4("model", model);

		glBindTexture(GL_TEXTURE_2D, depthMap);

		piso.Draw(Modellight);

		glfwSwapBuffers(janela);
		glfwPollEvents();
	}

	Shadow.nope();
	Modellight.nope();
	glfwTerminate();
}


void callback_redimensionamento(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
	largura = width;
	altura = height;
}  

void callback_cursor(GLFWwindow* janela, double xpos, double ypos){
	if (firstcursor) {
		lastx = xpos; lasty = ypos;
		firstcursor = false;
	}
	float deltax = xpos - lastx; float deltay = ypos - lasty;
	lastx = xpos; lasty = ypos;

	const float sensitivity = 0.05f;
	deltax *= sensitivity;
	deltay *= sensitivity;

	yaw -= deltax; pitch -= deltay;
	constrain(&pitch, 89.999f, -89.999f);
}

void constrain(float* value, float lim_up, float lim_down) {
	if (*value > lim_up) {
		*value = lim_up;
	}
	if (*value < lim_down) {
		*value = lim_down;
	}
}

void up_down(GLFWwindow* janela, float* var, float val, int key_up, int key_down, float lim_up, float lim_down) {
	if (glfwGetKey(janela, key_up) == GLFW_PRESS) {
		*var += val;
	}
	else if (glfwGetKey(janela, key_down) == GLFW_PRESS) {
		*var -= val;
	}
	constrain(var, lim_up, lim_down);
}

void callback_CloseWindow(GLFWwindow* janela) {
	if (glfwGetKey(janela, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(janela,true);
	}
}

void camera_movement(GLFWwindow* janela, float velocity, Camera viewer, glm::vec3 *position){
	if(glfwGetKey(janela, GLFW_KEY_D) == GLFW_PRESS){
		*position -= velocity * viewer.left;
	}
	else if(glfwGetKey(janela, GLFW_KEY_A) == GLFW_PRESS){
		*position += velocity * viewer.left;
	}

	if(glfwGetKey(janela, GLFW_KEY_W) == GLFW_PRESS){
		*position += velocity * viewer.front;
	}
	else if(glfwGetKey(janela, GLFW_KEY_S) == GLFW_PRESS){
		*position -= velocity * viewer.front;
	}

	if(glfwGetKey(janela, GLFW_KEY_Q) == GLFW_PRESS){
		*position += velocity * viewer.up;
	}
	else if(glfwGetKey(janela, GLFW_KEY_E) == GLFW_PRESS){
		*position -= velocity * viewer.up;
	}
}