#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "../../glaze/include/glaze.h" // TODO need to register this somehow, how?
#include <GLFW/glfw3.h>

#include "vertices.c"
#include "drawing.c"
#include "shaders.c"
#include "texture.c"
#include "window.c"

#ifndef MAX_OPEN_WINDOWS
	#define MAX_OPEN_WINDOWS 16
#endif




// TODO: put these in a config struct?
float zoom_incr = 0.0075;
float move_speed = 0.0075;


int window_width = 960;
int window_height = 960;

extern inline int idx(int x, int y, int z, int f, int size[3])
{
	int tmp = size[0] * size[1];
	return x + y * size[0] + z * tmp + f * tmp * size[2];
}

void get_ratio(int width, int height, float* p_ratio, int* p_ratio_axis)
{
	if (width > height) {
		*p_ratio = width / height;
		*p_ratio_axis = 0;
	}
	else {
		*p_ratio = height / width;
		*p_ratio_axis = 1;
	}
	return;
}

bool update_size(GLFWwindow* window, int* p_width, int* p_height, float* p_ratio, int* p_ratio_axis)
{
	int w, h;
	glfwGetWindowSize(window, &w, &h);
	if (*p_width != w || *p_height != h) {
		glViewport(0, 0, w, h);
		get_ratio(w, h, p_ratio, p_ratio_axis);
		*p_width = w;
		*p_height = h;
		return true;
	}
	return false;
}

void update_position(char plane, float planes[3][4][3], float centres[3][2], float centres_window[3][4], float mouse_window[2])
{
	// Change position of cross of plane
	for (int i = 0; i < 2; i++) centres_window[plane][i] = mouse_window[i];
	// Put tex coordinates into crosses of other planes
	window2tex(plane, centres_window[plane], centres[plane], planes[plane][0], planes[plane][2]);
	update_cross_centres(plane, centres);
	// Update window coordinates of other crosses
	for (int i = 0; i < 3; i++) {
		if (i == plane) continue;
		tex2window(i, centres[i], centres_window[i], planes[i][0], planes[i][2]);
	}
	// Update planes in other views
	update_view_slices(plane, planes, centres);
	return;
}


void update_zoom(float zoom, char plane, float planes[3][4][3], float centres[3][2], float centres_window[3][4], float mouse_tex[2])
{
	float shift[2] = {0};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 2; j++) {
			int axis = plane_axes[plane][j];
			float c = zoom * (planes[plane][i][axis] - mouse_tex[j]) + mouse_tex[j];
			planes[plane][i][axis] = c;
			if (c < 0)      c = -c;
			else if (c > 1) c = 1 - c;
			else            c = 0;
			if (c != 0)     shift[j] = c;
		}
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 2; j++) {
			int axis = plane_axes[plane][j];
			planes[plane][i][axis] = fmax(0, fmin(1, planes[plane][i][axis] + shift[j])); // Still need fmax/fmin because of inaccuracy
		}
	}
	tex2window(plane, centres[plane], centres_window[plane], planes[plane][0], planes[plane][2]);
	return;
}

bool handle_mouse_and_keys(GLFWwindow* window, int width, int height, float planes[3][4][3], float centres[3][2], float centres_window[3][4])
{
	char mouse_right_button_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
	char mouse_right_button	      = mouse_right_button_state                           == GLFW_PRESS;
	char mouse_left_button        = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	char zoom_in_key              = glfwGetKey(window, GLFW_KEY_EQUAL)                 == GLFW_PRESS;
	char zoom_out_key             = glfwGetKey(window, GLFW_KEY_MINUS)                 == GLFW_PRESS;
	char move_up_key              = glfwGetKey(window, GLFW_KEY_UP)                    == GLFW_PRESS;
	char move_down_key            = glfwGetKey(window, GLFW_KEY_DOWN)                  == GLFW_PRESS;
	char move_right_key           = glfwGetKey(window, GLFW_KEY_RIGHT)                 == GLFW_PRESS;
	char move_left_key            = glfwGetKey(window, GLFW_KEY_LEFT)                  == GLFW_PRESS;

	static char clicked_plane = -1;
	//printf("%d\n", clicked_plane);
	if (mouse_right_button_state == GLFW_RELEASE) clicked_plane = -1;

	bool any_key_pressed = (
		mouse_left_button || mouse_right_button ||
		zoom_in_key       || zoom_out_key  ||
		move_up_key       || move_down_key || move_right_key || move_left_key
	);
	if (!any_key_pressed) return false; 

	float mouse_window[2];
	float mouse_delta[2];
	update_mouse_position(window, width, height, mouse_window, mouse_delta); // TODO maybe this can be merged with the effect of mouse_delta? check below what it does
	float mouse_tex[2];
	char plane = current_plane(mouse_window[0], mouse_window[1]);

	// Is mouse in a plane?
	if (plane != -1) {
		window2tex(plane, mouse_window, mouse_tex, planes[plane][0], planes[plane][2]);
		if (mouse_left_button) update_position(plane, planes, centres, centres_window, mouse_window);
		if (zoom_in_key || zoom_out_key) {
			float zoom = 1;
			if (zoom_in_key)	zoom -= zoom_incr;
			else if (zoom_out_key)	zoom += zoom_incr;
			update_zoom(zoom, plane, planes, centres, centres_window, mouse_tex);
		}
		if (move_up_key || move_down_key || move_right_key || move_left_key) {
			char sign = move_up_key || move_right_key ? 1 : -1;
			char axis = move_up_key || move_down_key ? 1 : 0;
			float rel_shift[2] = {0};
			rel_shift[axis] = sign * move_speed;
			move_view(rel_shift, plane, planes[plane], centres[plane], centres_window[plane]);
		}
		if (clicked_plane == -1) clicked_plane = plane;
	}


	// Was mouse previously clicked in a plane and right mouse button is still held?
	if (clicked_plane != -1 && mouse_right_button) {
		float rel_shift[2];
		for (int i = 0; i < 2; i++) rel_shift[i] = 100 * mouse_delta[i] * move_speed;
		move_view(rel_shift, clicked_plane, planes[clicked_plane], centres[clicked_plane], centres_window[clicked_plane]);
	}

	return true;
}




void poirot(int width, int height)
{ // float *image, int size[3], int nframes, 

	int nframes = 1;
	int size[3] = {100, 100, 100};

	float image[100 * 100 * 100] = { 0 };
	for (int i = 49; i < 52; i++) {
		for (int j = 49; j < 52; j++) {
			for (int k = 49; k < 52; k++) {
				image[idx(i, j, k, 0, size)] = 1;
			}
		}
	}

	GLFWwindow* window = open_window(width, height);
	// TODO break this down?
	gladLoadGL();
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(glErrorCallback, NULL);
	glClearColor(0, 0, 0, 1);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glLineWidth(2.0);

	bool update = false;
	int ratio_axis;
	float ratio;
	get_ratio(width, height, &ratio, &ratio_axis);

	// TODO outsource this?
	GLuint plane_program, plane_shaders[2], cross_program, cross_shaders[2];
	setup_plane_shaders(&plane_program, plane_shaders);
	setup_cross_shaders(&cross_program, cross_shaders);

	GLint uniform_ratio = glGetUniformLocation(plane_program, "ratio");
	GLint uniform_ratio_axis = glGetUniformLocation(plane_program, "axis");
	GLint cross_vertical = glGetUniformLocation(cross_program, "vertical");

	glUseProgram(plane_program);
	glUniform1f(uniform_ratio, ratio);
	glUniform1i(uniform_ratio_axis, ratio_axis);

	#include "coordinates.c"

	GLuint buffers[2];
	glGenBuffers(2, &buffers[0]);

	// TODO: single plane, think I need only one for all three orientations?
	GLuint three_planes_vertex_array = setup_three_planes(buffers[0], planes);
	GLuint crosses_vertex_array = setup_crosses(buffers[1], centres_window);
	GLuint texture = setup_texture(image, size);

	while (!glfwWindowShouldClose(window)) {

		//glfwPollEvents();
		bool ...
		while (
			....
			update = update_size(window, &width, &height, &ratio, &ratio_axis);
			... = handle_mouse_and_keys(window, width, height, planes, centres, centres_window)
			esc = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS
		)
			glfwWaitEvents();
		}
		if (esc) break;


		if (... check below, crosses updated there as well) {
			glClear(GL_COLOR_BUFFER_BIT);
			glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(crosses), sizeof(centres_window), centres_window);
		}
		//printf("%f, %f\n", centres_window[0][0], centres_window[0][1]);

		// TODO outsource all this into a drawing function?
		glUseProgram(plane_program);
		glBindVertexArray(three_planes_vertex_array);

		if (update) {
			// TODO don't copy all?
			glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(three_planes_vertices), sizeof(planes), planes);
			glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(crosses), sizeof(centres_window), centres_window);
			glUniform1f(uniform_ratio, ratio);
			update = false;
		}

		// Draw: TODO: when do I have to redraw, also don't swapp buffers in that case
		for (int i = 0; i <= 8; i += 4) {
			glUniform1i(uniform_ratio_axis, plane_axes[i / 4][ratio_axis]);
			glDrawArrays(GL_TRIANGLE_FAN, i, 4);
		}

		// Crosses
		glUseProgram(cross_program);
		glBindVertexArray(crosses_vertex_array);
		glUniform1i(cross_vertical, 0);
		glDrawArraysInstanced(GL_LINES, 0, 2, 3);
		glUniform1i(cross_vertical, 1);
		glDrawArraysInstanced(GL_LINES, 2, 2, 3);

		// Flush and swap
		glFlush();
		glfwSwapBuffers(window);
	}

	// Clean up
	glfwDestroyWindow(window);

	return;
}



void poirot_done()
{
	// TODO: clean anything else shaders and program
	glfwTerminate();
	return;
}



int main(int argc, char* argv[])
{
	if (!glfwInit()) {
		printf("Error: could not initialise GLFW");
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwSetErrorCallback(error_callback);

	poirot(800, 600);

	poirot_done();

	return 0;
}

