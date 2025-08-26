#ifndef DECKIFY_H
#define DECKIFY_H

#include <stdio.h>
#include <winsock2.h>
#include <ws2bth.h>
#include <bluetoothapis.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include "external/glew-2.1.0/include/GL/glew.h"
#include "external/glfw-3.4.bin.WIN64/include/GLFW/glfw3.h"
#include "external/freetype/ft2build.h" 
#include FT_FREETYPE_H

#define CURSOR_SIZE 10
#define BITMAP_SIZE 2000

typedef struct Glyph {
	GLuint textureID;
	int width;
	int height;
	int bearingX;
	int bearingY;
	unsigned int advance;
} Glyph;

typedef struct Font {
	Glyph* glyphs;
	int fontSize;
	int lineHeight;
	int ascender;
	int descender;
} Font;

typedef struct textRenderParameters {
	GLuint shaderProgram;
	Font font;
	const char* text;
	float x;
	float y;
	float scale;
	float color[3];
} textRenderParameters;

typedef struct textureRenderParameters {
	GLuint shaderProgram;
	float x;
	float y;
	int width;
	int height;
	float color[3];
	GLuint textureID;
} textureRenderParameters;

typedef struct Widget {
	int x;
	int y;
	int width;
	int height;
	float scale;
	float color[3];
	textRenderParameters textParams;
	textureRenderParameters textureParams;
	void (*onRender)(struct Widget* self);
	void (*onHover)(struct Widget* self);
	void (*onClick)(struct Widget* self);
} Widget;

typedef struct point {
	float x, y;
	float pressure;
} Point;

typedef struct line {
	Point start;
	Point end;
} Line;

typedef struct queue {
	Line val;
	int mode;
	struct queue *next;
} LineQueue;

typedef struct listenerParams {
	SOCKET socket;
	void (*data_callback)(const char*, int);
} ListenerParams;

typedef struct appState {
	SOCKET bluetooth_socket;
	int screenWidth;
	int screenHeight;
	int penPosition[2];
	int mode;
	LineQueue *queue;
	GLuint *textures;
	GLuint lineShaderProgram;
	GLuint eraseShaderProgram;
	GLuint textureShaderProgram;
} AppState;

GLFWwindow *openWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share);
void closeWindow(GLFWwindow* window);

int initializeBluetooth();
int discoverDeviceByName(const wchar_t *device_name, BLUETOOTH_ADDRESS *out_addr);
SOCKET connectToDeviceWithUUID(BLUETOOTH_ADDRESS btAddr, GUID service_uuid);
int startBluetoothListener(ListenerParams *params);
int connectBluetooth(const wchar_t *name, const char *uuidString, SOCKET *socket, void (*handleData)(const char*, int));
void cleanupBluetooth(SOCKET bluetooth_socket);

void errorCallback(int error, const char* description);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

void enqueueLine(Point start, Point end, int mode, LineQueue **queue);
void dequeueLine(LineQueue **queue);
int queueLength(LineQueue **queue);
int renderCanvas(AppState *state);

void renderTexture(textureRenderParameters params);
void renderLine(GLuint lineShaderProgram, GLuint bitmap, Point start, Point end, float thickness);
void eraseBitmap(GLuint eraseShaderProgram, GLuint bitmap, Point start, Point end);

GLuint loadShader(const char* vertexPath, const char* fragmentPath);
GLuint loadTexture(const char* filename);
char* readFile(const char* filename);
GLuint generateBitmap(int width, int height, unsigned char* data);
void resizeTexture(GLuint *oldTexture, int oldWidth, int oldHeight, int newWidth, int newHeight);

#endif