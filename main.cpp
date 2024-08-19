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

	float vertices[] = {
        // positions          // colors           // texture coords
         1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
         1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	Shader Modellight("../Shaders/AssimpExample.vert", "../Shaders/AssimpExample.frag");
	Shader Shadow("../Shaders/Shadow.vert", "../Shaders/Shadow.frag");
	Shader gDeferred("../Shaders/gDeferred.vert", "../Shaders/gDeferred.frag");
	Shader lDeferred("../Shaders/lDeferred.vert", "../Shaders/lDeferred.frag");
	glEnable(GL_DEPTH_TEST);

	float angle = 0.0f;
	float angle_var = 0.0f;

	double time = 0;
	
	//Shadow Map
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

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

	float near_plane = 0.1f, far_plane = 7.5f;
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

	Camera dir_light(glm::vec3(1.0f, 1.0f, -1.5f), 
					glm::vec3(1.0f, 4.0f, 2.0f));
	
	glm::mat4 lightView = dir_light.view;

	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	Model parede("../models/parede/parede.obj");
	Model lixo("../models/lixo/lixo.obj");
	Model piso("../models/piso/piso.obj");

	glm::vec3 pos = glm::vec3(0.0f, 1.0f, 5.0f);
	glm::vec4 color;

	//deferred shading

	unsigned int gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	unsigned int gPosition, gNormal, gAlbedoSpec;
	
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, largura, altura, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
	
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, largura, altura, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
	
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, largura, altura, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
	
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, largura, altura);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Falta algo ao framebuffer" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

	while (!glfwWindowShouldClose(janela)){
		callback_CloseWindow(janela);

		glCullFace(GL_FRONT);
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

		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

		glCullFace(GL_BACK);

		glViewport(0, 0, largura, altura);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gDeferred.use();

		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(45.0f), (GLfloat)(largura)/(GLfloat)(altura), 0.01f, 50.0f);
		gDeferred.setmat4("projection", projection);

		Camera viewer = Camera(pos, yaw, pitch);
		float velocity = 5*(glfwGetTime() - time);
		time = glfwGetTime();
		camera_movement(janela, velocity, viewer, &pos);
		gDeferred.setmat4("view", viewer.view);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -3.0f));
		model = glm::scale(model, glm::vec3(1.0, 1.0, 1.0));
		gDeferred.setmat4("model", model);
		parede.Draw(gDeferred);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.5f));
		model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
		gDeferred.setmat4("model", model);
		lixo.Draw(gDeferred);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.5f));
		gDeferred.setmat4("model", model);
		piso.Draw(gDeferred);


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		lDeferred.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		lDeferred.setint("gPosition", 0);
		lDeferred.setint("gNormal", 1);
		lDeferred.setint("gAlbedoSpec", 2);

		lDeferred.setvec3("light.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
		lDeferred.setvec3("light.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        lDeferred.setvec3("light.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
        lDeferred.setvec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		Modellight.setfloat("material.shininess", 32);
		lDeferred.setvec3("viewPos", viewer.position);
		lDeferred.setmat4("lightSpaceMatrix", lightSpaceMatrix);

		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		Modellight.setint("shadowMap", 8);


		glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);


		glfwSwapBuffers(janela);
		glfwPollEvents();
	}

	gDeferred.nope();
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