#include "../libs/deckify.h"

void errorCallback(int error, const char* description) {
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void framebufferSizeCallback(GLFWwindow* window, int newwidth, int newheight) {
	glViewport(0, 0, newwidth, newheight);
	AppState *state = (AppState*)glfwGetWindowUserPointer(window);
	state->screenWidth = newwidth;
	state->screenHeight = newheight;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	AppState *state = (AppState*)glfwGetWindowUserPointer(window);
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}