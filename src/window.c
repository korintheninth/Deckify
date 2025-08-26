#include "../libs/deckify.h"

GLFWwindow* openWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share) {
	if (!glfwInit()) {
		printf("Failed to initialize GLFW\n");
		return NULL;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, monitor, share);
	if (!window) {
		printf("Failed to create window\n");
		glfwTerminate();
		return NULL;
	}
	
	glfwMakeContextCurrent(window);
	glfwSetErrorCallback(errorCallback);
	
	if (glewInit() != GLEW_OK) {
		printf("Failed to initialize GLEW\n");
		return NULL;
	}
	return window;
}

void closeWindow(GLFWwindow* window) {
	glfwDestroyWindow(window);
	glfwTerminate();
}