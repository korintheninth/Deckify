#include "../libs/deckify.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../libs/external/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../libs/external/stb_image_resize2.h"

char* readFile(const char* filename) {
	FILE* file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "Could not open file: %s\n", filename);
		return NULL;
	}

	// Seek to the end to determine the file size
	if (fseek(file, 0, SEEK_END) != 0) {
		fprintf(stderr, "Failed to seek to end of file: %s\n", filename);
		fclose(file);
		return NULL;
	}

	long length = ftell(file);
	if (length < 0) {
		fprintf(stderr, "Failed to determine file size: %s\n", filename);
		fclose(file);
		return NULL;
	}

	if (fseek(file, 0, SEEK_SET) != 0) {
		fprintf(stderr, "Failed to seek to beginning of file: %s\n", filename);
		fclose(file);
		return NULL;
	}

	char* buffer = malloc(length + 1);
	if (!buffer) {
		fprintf(stderr, "Failed to allocate memory for file content\n");
		fclose(file);
		return NULL;
	}

	size_t readLength = fread(buffer, 1, length, file);
	if (readLength != length) {
		fprintf(stderr, "Failed to read the entire file: %s\n", filename);
		free(buffer);
		fclose(file);
		return NULL;
	}

	buffer[length] = '\0';

	fclose(file);
	return buffer;
}

GLuint loadShader(const char* vertexPath, const char* fragmentPath) {
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint shaderProgram = glCreateProgram();
	
	char* vertexCode = readFile(vertexPath);
	glShaderSource(vertexShader, 1, (const char**)&vertexCode, NULL);
	glCompileShader(vertexShader);
	
	GLint success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n%s\n\n", infoLog, vertexPath);
	}

	char* fragmentCode = readFile(fragmentPath);
	glShaderSource(fragmentShader, 1, (const char**)&fragmentCode, NULL);
	glCompileShader(fragmentShader);
	
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n%s\n\n", infoLog, fragmentPath);
	}

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
	}

	free(vertexCode);
	free(fragmentCode);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

GLuint loadTexture(const char* filename) {
	int width, height, channels;
	
	stbi_set_flip_vertically_on_load(1);
	
	unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
	if (!data) {
		fprintf(stderr, "Failed to load image: %s\n", filename);
		return 0;
	}

	GLenum format;
	if (channels == 1)
		format = GL_RED;
	else if (channels == 3)
		format = GL_RGB;
	else if (channels == 4)
		format = GL_RGBA;
	else {
		fprintf(stderr, "Unsupported number of channels: %d\n", channels);
		stbi_image_free(data);
		return 0;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		format,
		width,
		height,
		0,
		format,
		GL_UNSIGNED_BYTE,
		data
	);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		fprintf(stderr, "OpenGL error: %d\n", error);
		glDeleteTextures(1, &texture);
		stbi_image_free(data);
		return 0;
	}

	stbi_image_free(data);

	return texture;
}

GLuint generateBitmap(int width, int height, unsigned char* data) {
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

void resizeTexture(GLuint *oldTexture, int oldWidth, int oldHeight, int newWidth, int newHeight) {
	GLint currentFBO;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
	GLint currentTexture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTexture);
	
	GLuint newTexture;
	glGenTextures(1, &newTexture);
	glBindTexture(GL_TEXTURE_2D, newTexture);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newWidth, newHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GLuint srcFBO, dstFBO;
	glGenFramebuffers(1, &srcFBO);
	glGenFramebuffers(1, &dstFBO);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFBO);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *oldTexture, 0);
	
	if (glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "Source framebuffer not complete\n");
		goto cleanup;
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFBO);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newTexture, 0);
	
	if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "Destination framebuffer not complete\n");
		goto cleanup;
	}
	
	int copyWidth = (oldWidth < newWidth) ? oldWidth : newWidth;
	int copyHeight = (oldHeight < newHeight) ? oldHeight : newHeight;
	
	glBlitFramebuffer(
		0, 0, copyWidth, copyHeight,
		0, 0, copyWidth, copyHeight,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR
	);

cleanup:
	glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
	glBindTexture(GL_TEXTURE_2D, currentTexture);

	glDeleteFramebuffers(1, &srcFBO);
	glDeleteFramebuffers(1, &dstFBO);
	glDeleteTextures(1, oldTexture);
	*oldTexture = newTexture;
}