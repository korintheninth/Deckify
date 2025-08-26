#define _USE_MATH_DEFINES
#include "../libs/deckify.h"

void renderTexture(textureRenderParameters params) {
	GLuint VAO, VBO;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glUseProgram(params.shaderProgram);

	int width, height;
	glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
	float screenSize[] = { (float)width, (float)height };
	glUniform2fv(glGetUniformLocation(params.shaderProgram, "screenSize"), 1, screenSize);
	glUniform3fv(glGetUniformLocation(params.shaderProgram, "textureColor"), 1, params.color);

	glActiveTexture(GL_TEXTURE0);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



	float xpos = params.x;
	float ypos = params.y;

	float w = params.width;
	float h = params.height;

	GLfloat vertices[6][4] = {
		{ xpos,     ypos + h,   0.0f, 0.0f },
		{ xpos + w, ypos,       1.0f, 1.0f },
		{ xpos,     ypos,       0.0f, 1.0f },

		{ xpos,     ypos + h,   0.0f, 0.0f },
		{ xpos + w, ypos + h,   1.0f, 0.0f },
		{ xpos + w, ypos,       1.0f, 1.0f }
	};
		
	glBindTexture(GL_TEXTURE_2D, params.textureID);


	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
	glUseProgram(0);
}

void renderLine(GLuint lineShaderProgram, GLuint bitmap, Point start, Point end, float thickness) {
	glUseProgram(lineShaderProgram);

	glUniform3fv(glGetUniformLocation(lineShaderProgram, "lineColor"), 1, (GLfloat[]){1.0f, 0.0f, 0.0f});

	int width, height;
	glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
	glUniform2f(glGetUniformLocation(lineShaderProgram, "screenSize"), (float)width, (float)height);

	GLuint FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bitmap, 0);
	
	GLenum drawBuf = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, &drawBuf);
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer not complete!\n");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &FBO);
		return;
	}

	float dx = end.x - start.x;
	float dy = end.y - start.y;
	float length = sqrt(dx * dx + dy * dy);
	
	if (length > 0) {
		float perpX = -dy / length * thickness * 0.5f;
		float perpY = dx / length * thickness * 0.5f;
		
		Point lineVertices[6] = {
			{start.x + perpX, start.y + perpY},  // Top-left
			{start.x - perpX, start.y - perpY},  // Bottom-left
			{end.x + perpX, end.y + perpY},      // Top-right
			
			{start.x - perpX, start.y - perpY},  // Bottom-left
			{end.x - perpX, end.y - perpY},      // Bottom-right
			{end.x + perpX, end.y + perpY}       // Top-right
		};

		GLuint VAO, VBO;
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Point), lineVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
		glEnableVertexAttribArray(0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		glDisableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);
	}

	const int circleSegments = 16;
	float radius = thickness * 0.5f;
	
	for (int cap = 0; cap < 2; cap++) {
		Point center = (cap == 0) ? start : end;
		
		Point circleVertices[circleSegments * 3];
		
		for (int i = 0; i < circleSegments; i++) {
			float angle1 = (float)i / circleSegments * 2.0f * M_PI;
			float angle2 = (float)(i + 1) / circleSegments * 2.0f * M_PI;
			
			circleVertices[i * 3] = (Point){center.x, center.y};
			circleVertices[i * 3 + 1] = (Point){
				center.x + cos(angle1) * radius,
				center.y + sin(angle1) * radius
			};
			circleVertices[i * 3 + 2] = (Point){
				center.x + cos(angle2) * radius,
				center.y + sin(angle2) * radius
			};
		}

		GLuint VAO, VBO;
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, circleSegments * 3 * sizeof(Point), circleVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
		glEnableVertexAttribArray(0);

		glDrawArrays(GL_TRIANGLES, 0, circleSegments * 3);
		
		glDisableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);
	}

	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &FBO);
}

void eraseBitmap(GLuint eraseShaderProgram, GLuint bitmap, Point start, Point end) {
	glBindTexture(GL_TEXTURE_2D, bitmap);
	glClearTexSubImage(
		bitmap, 0,
		(int)start.x, (int)start.y, 0,
		(int)(end.x - start.x), (int)(end.y - start.y), 1,
		GL_RGBA, GL_UNSIGNED_BYTE, NULL
	);
	glBindTexture(GL_TEXTURE_2D, 0);
}