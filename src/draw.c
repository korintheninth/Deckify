#include "../libs/deckify.h"

void enqueueLine(Point start, Point end, int mode, LineQueue **queue) {
	if (start.x == end.x && start.y == end.y) return;

	LineQueue *newLine = malloc(sizeof(LineQueue));
	if (!newLine) {
		fprintf(stderr, "Failed to allocate memory for new line\n");
		return;
	}
	newLine->val.start = start;
	newLine->val.end = end;
	newLine->mode = mode;
	newLine->next = NULL;

	if (*queue == NULL) {
		*queue = newLine;
	} else {
		LineQueue *current = *queue;
		while (current->next != NULL) {
			current = current->next;
		}
		current->next = newLine;
	}
}

void dequeueLine(LineQueue **queue) {
	if (queue == NULL || *queue == NULL) return;

	LineQueue *temp = *queue;
	*queue = (*queue)->next;
	free(temp);

}

int queueLength(LineQueue **queue) {
	int length = 0;
	LineQueue *current = *queue;
	while (current != NULL) {
		length++;
		current = current->next;
	}
	return length;
}

int renderCanvas(AppState *state) {
	while (state->queue) {
		Line line = state->queue->val;
		if (state->queue->mode == 1) {
			glBlendFunc(GL_ZERO, GL_ZERO);
			renderLine(state->eraseShaderProgram, state->textures[0], line.start, line.end, 50.0f);
		} else if (state->queue->mode == 0) {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			renderLine(state->lineShaderProgram, state->textures[0], line.start, line.end,
					(line.start.pressure + line.end.pressure) / 2.0f * 10.0f);
		}
		dequeueLine(&state->queue);
	}
	textureRenderParameters params = {
		state->textureShaderProgram,
		0,
		state->screenHeight - BITMAP_SIZE,
		BITMAP_SIZE,
		BITMAP_SIZE,
		{1.0f, 1.0f, 1.0f},
		state->textures[0]
	};
	renderTexture(params);
	params = (textureRenderParameters){
		state->textureShaderProgram,
		state->penPosition[0] - ((state->mode == 0) ? CURSOR_SIZE : CURSOR_SIZE * 5) / 2,
		state->penPosition[1] - ((state->mode == 0) ? CURSOR_SIZE : CURSOR_SIZE * 5) / 2,
		(state->mode == 0) ? CURSOR_SIZE : CURSOR_SIZE * 5,
		(state->mode == 0) ? CURSOR_SIZE : CURSOR_SIZE * 5,
		{1.0f, 1.0f, 1.0f},
		(state->mode == 0) ? state->textures[1] : state->textures[2]
	};
	renderTexture(params);
}