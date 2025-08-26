#include "../libs/deckify.h"

AppState state;

void handleData(const char *data, int len) {
	if (len < 1) {
		printf("Received empty data\n");
		return;
	}

	static float prevPosx, prevPosy, prevPressure = 0.0f;
	int action = atoi(strtok((char*)data, " "));
	float x = atof(strtok(NULL, " "));
	float y = atof(strtok(NULL, " "));
	float pressure = atof(strtok(NULL, " "));
	if (pressure < 0.0f)
		state.mode = 1;
	else
		state.mode = 0;
	state.penPosition[0] = x;
	state.penPosition[1] = state.screenHeight - y;
	if (action == 2) {
		state.penPosition[0] = 0;
		state.penPosition[1] = 0;
	}
	if (action == 4) {
		enqueueLine((Point){prevPosx, prevPosy, prevPressure}, (Point){state.penPosition[0], state.penPosition[1], pressure}, state.mode, &state.queue);
	}
	prevPressure = pressure;
	prevPosx = state.penPosition[0];
	prevPosy = state.penPosition[1];
}

int main() {
	state = (AppState) {
		.screenWidth = 800,
		.screenHeight = 600,
		.penPosition = {0, 0},
		.mode = 0,
		.queue = NULL
	};

	if (connectBluetooth(L"Skibidy Toilet (Galaxy Note3)", "248dd985-12af-420b-b46b-fd76109a64e9", &(state.bluetooth_socket), handleData) != 0) {
		printf("Failed to connect to Bluetooth device\n");
		return -1;
	}

	GLFWwindow* window = openWindow(state.screenWidth, state.screenHeight, "Deckify", NULL, NULL);
	if (!window) {
		return -1;
	}

	glfwSetWindowUserPointer(window, &state);
	
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetErrorCallback(errorCallback);
	glfwMaximizeWindow(window);

	state.textures = malloc(sizeof(GLuint) * 3);
	state.textures[0] = generateBitmap(BITMAP_SIZE, BITMAP_SIZE, NULL);
	state.textures[1] = loadTexture("textures/cursor.png");
	state.textures[2] = loadTexture("textures/eraser.png");

	state.textureShaderProgram = loadShader("src/shaders/texturevertexshader.glsl", "src/shaders/texturefragmentshader.glsl");
	state.lineShaderProgram = loadShader("src/shaders/linevertexshader.glsl", "src/shaders/linefragmentshader.glsl");
	state.eraseShaderProgram = loadShader("src/shaders/linevertexshader.glsl", "src/shaders/erasefragmentshader.glsl");
	
	glEnable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	
	while (!glfwWindowShouldClose(window)) {
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		renderCanvas(&state);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	cleanupBluetooth(state.bluetooth_socket);
	glfwDestroyWindow(window);
	glDeleteProgram(state.lineShaderProgram);
	glDeleteProgram(state.eraseShaderProgram);
	glDeleteProgram(state.textureShaderProgram);
	glDeleteTextures(3, state.textures);
	free(state.textures);
	glfwTerminate();
	return 0;
}