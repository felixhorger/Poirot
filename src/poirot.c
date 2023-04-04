#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "../include/glad.h"
#include <GLFW/glfw3.h>


void error_callback(int error, const char* description) {
	fprintf(stderr, "Error: GLFW: %s\n", description);
	exit(1);
	return;
}

void GLAPIENTRY gl_error_callback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam
) {
	printf(
		"Error: OpenGL: type = 0x%x, severity = 0x%x, message = %s\n",
		type, severity, message
	);
	exit(1);
}

int window_length, window_offset_x, window_offset_y;
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	window_length = width < height ? width : height;
	window_offset_x = (width - window_length) / 2;
	window_offset_y = (height - window_length) / 2;
	glViewport(window_offset_x, window_offset_y, window_length, window_length);
	return;
}

void check_program(
	GLuint program,
	GLenum pname,
	void (*glGetiv)(GLuint, GLenum, GLint*),
	void (*glGetInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*),
	void (*glDelete)(GLuint),
	char *msg
) {
	GLuint result;
	glGetiv(program, pname, &result);
	if (result == GL_FALSE) {
		GLint length; // includes null char
		glGetiv(program, GL_INFO_LOG_LENGTH, &length);
		if (length > 0) {
			char *error = malloc(length);
			glGetInfoLog(program, length, NULL, error);
			printf("Error: %s\n%s\n", msg, error);
			free(error);
			glDelete(program);
			exit(1);
		}
	}
	return;
}

void create_program(const char *shader_code[2], GLuint *p_program, GLuint shaders[2]) {
	GLuint program = glCreateProgram();
	// Compile shaders and attach to program
	const GLenum shader_type[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
	for (int i = 0; i < 2; i++) {
		GLuint shader;
		shader = glCreateShader(shader_type[i]);
		glShaderSource(shader, 1, &shader_code[i], NULL);
		glCompileShader(shader);
		check_program(shader, GL_COMPILE_STATUS, glGetShaderiv, glGetShaderInfoLog, glDeleteShader, "Could not compile shader");
		glAttachShader(program, shader);
		shaders[i] = shader;
	}
	// Finalise program
	glLinkProgram(program);
	check_program(program, GL_LINK_STATUS, glGetProgramiv, glGetProgramInfoLog, glDeleteProgram, "Could not link program");
	glValidateProgram(program);
	check_program(program, GL_VALIDATE_STATUS, glGetProgramiv, glGetProgramInfoLog, glDeleteProgram, "Program invalid");
	*p_program = program;
	return;
}
// TODO: delete shader properly at end


char active_plane(double x, double y) {
	int plane = -1;
	if (x >= -0.98 && x <= -0.02) {
		if (y >= 0.02 && y <= 0.98)			plane = 0; // (0, 1, 2)
		else if (y <= -0.02 && y >= -0.98)	plane = 1; // (0, 2, 1)
	}
	else if (x >= 0.02 && x <= 0.98 && y >= 0.02 && y <= 0.98) plane = 2; // (1, 2, 0)
	return plane;
}

void plane_0_tex_coords(float tex_coord[2], float window_coord[2], float tex_lower[2], float tex_upper[2]) {
	tex_coord[0] = (window_coord[0] - 0.02 + 1.0) / 0.96 * (tex_upper[1] - tex_lower[1]) + tex_lower[1];
	tex_coord[1] = (window_coord[1] - 0.02      ) / 0.96 * (tex_upper[0] - tex_lower[0]) + tex_lower[0];
	return;
}

void plane_0_window_coords(float window_coord[2], float tex_coord[2], float tex_lower[2], float tex_upper[2]) {
	window_coord[0] = (window_coord[0] - tex_lower[1]) * 0.96 / (tex_upper[1] - tex_lower[1]) + 0.02 - 1.0;
	window_coord[1] = (window_coord[1] - tex_lower[0]) * 0.96 / (tex_upper[0] - tex_lower[0]) + 0.02;
	return;
}



int main(int argc, char* argv[]) {

	// Open window
	if (!glfwInit()) {
		printf("Error: could not initialise GLFW");
		return 1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwSetErrorCallback(error_callback);
	window_length = 800;
	window_offset_x = 0;
	window_offset_y = 0;
	GLFWwindow* window = glfwCreateWindow(window_length, window_length, "", NULL, NULL); //glfwGetPrimaryMonitor()
	if (!window) {
		printf("Error: could not open window");
		return 1;
	}
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	gladLoadGL();
	glViewport(0, 0, window_length, window_length);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(gl_error_callback, 0);

	// Views shaders
	GLuint views_program, views_shaders[2];
	{
		const char *shader_code[] = {
			"\
				#version 450 core											\n\
				layout (location = 0) in vec2 position;						\n\
				layout (location = 1) in vec3 in_tex_coordinate;			\n\
				out vec3 tex_coordinate;									\n\
				void main() {												\n\
					gl_Position = vec4(position, 0.0, 1.0);					\n\
					tex_coordinate = in_tex_coordinate;						\n\
				}															\n\
			",
			"\
				#version 450 core										\n\
				uniform sampler3D tex;									\n\
				in vec3 tex_coordinate;									\n\
				out vec4 colour;										\n\
				void main(void) {										\n\
					colour = texture(tex, tex_coordinate);				\n\
				}														\n\
			"
		};
		create_program(shader_code, &views_program, views_shaders);
	}
	const float views[] = {
		 // (0, 1, 2)
		-0.98f,  0.02f,
		-0.02f,  0.02f,
		-0.02f,  0.98f,
		-0.98f,  0.98f,
		 // (0, 2, 1)
		-0.98f, -0.98f,
		-0.02f, -0.98f,
		-0.02f, -0.02f,
		-0.98f, -0.02f,
		 // (2, 1, 0)
		 0.02f,  0.02f,
		 0.98f,  0.02f,
		 0.98f,  0.98f,
		 0.02f,  0.98f
	};
	float tex_coords[3][4][3] = { // plane, corner, axis
		{ // (0, 1, 2)
			{ 0.0f, 0.0f, 0.5f },
			{ 0.0f, 1.0f, 0.5f },
			{ 1.0f, 1.0f, 0.5f },
			{ 1.0f, 0.0f, 0.5f }
		},
		{ // (0, 2, 1)
			{ 0.0f, 0.5f, 0.0f },
			{ 0.0f, 0.5f, 1.0f },
			{ 1.0f, 0.5f, 1.0f },
			{ 1.0f, 0.5f, 0.0f }
		},
		{ // (2, 1, 0)
			{ 0.5f, 0.0f, 0.0f },
			{ 0.5f, 1.0f, 0.0f },
			{ 0.5f, 1.0f, 1.0f },
			{ 0.5f, 0.0f, 1.0f }
		}
	};

	// Cross shaders
	GLuint crosses_program, crosses_shaders[2];
	{
		const char *shader_code[] = {
			"\
				#version 450 core								\n\
				layout (location = 0) in vec2 in_position;			\n\
				layout (location = 1) in vec4 centre;			\n\
				uniform bool vertical;			\n\
				out vec2 position;					\n\
				out vec2 offset;					\n\
				void main() {									\n\
					if (vertical) {            \n\
						if (abs(centre[0] - centre[2]) > 0.48) { \n\
							gl_Position = vec4(2.0, 0.0, 0.0, 1.0);		\n\
							return; \n\
						} \n\
						offset[0] = centre[0];        \n\
						offset[1] = centre[3];        \n\
					}                             \n\
					else {                         \n\
						if (abs(centre[1] - centre[3]) > 0.48) { \n\
							gl_Position = vec4(2.0, 0.0, 0.0, 1.0);		\n\
							return; \n\
						} \n\
						offset[0] = centre[2]; \n\
						offset[1] = centre[1];        \n\
					}                               \n\
					gl_Position = vec4(in_position + offset, 0.0, 1.0);		\n\
					position = gl_Position.xy; \n\
				}												\n\
			",
			"\
				#version 450 core							\n\
				in vec2 position;					\n\
				in vec2 offset;					\n\
				out vec4 colour; \n\
				void main(void) {							\n\
					if (									\n\
						abs(position[0] - offset[0]) <= 0.48 &&   \n\
						abs(position[1] - offset[1]) <= 0.48       \n\
					)		colour = vec4(0.0, 1.0, 0.0, 1.0);		\n\
					else	discard;		\n\
				}											\n\
			"
		};
		create_program(shader_code, &crosses_program, crosses_shaders);
	}
	const float crosses[] = {
		-0.48f,  0.00f,
		 0.48f,  0.00f,
		 0.00f, -0.48f,
		 0.00f,  0.48f
	};
	float centres[3][4] = {
		{-0.5f,  0.5f, -0.5f,  0.5f}, // (0, 1, 2)
		{-0.5f, -0.5f, -0.5f, -0.5f}, // (0, 2, 1)
		{ 0.5f,  0.5f,  0.5f,  0.5f}  // (1, 2, 0)
	};

	float image[100*100*100] = { 0 };

	GLuint buffers[2];
	glGenBuffers(2, &buffers[0]);
	// View
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(views)+sizeof(tex_coords), NULL, GL_DYNAMIC_STORAGE_BIT);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(views), views);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(views), sizeof(tex_coords), tex_coords);
	//
	GLuint views_vertex_array;
	glGenVertexArrays(1, &views_vertex_array);
	glBindVertexArray(views_vertex_array);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)sizeof(views));
	// Crosses
	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(crosses)+sizeof(centres), NULL, GL_DYNAMIC_STORAGE_BIT);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(crosses), crosses);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(crosses), sizeof(centres), centres);
	// 
	GLuint crosses_vertex_array;
	glGenVertexArrays(1, &crosses_vertex_array);
	glBindVertexArray(crosses_vertex_array);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void *)(sizeof(crosses)));
	glVertexAttribDivisor(1, 1);
	// Uniform to indicate if vertical branches are drawn
	GLint cross_vertical = glGetUniformLocation(crosses_program, "vertical");
	// Texture
	GLuint texture;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_3D, texture);
	glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32F, 100, 100, 100);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	{
		long rst_coordinates[3] = {GL_TEXTURE_WRAP_R, GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T};
		for (int i = 0; i < 3; i++) glTexParameteri(GL_TEXTURE_3D, rst_coordinates[i], GL_CLAMP_TO_BORDER);
		const float zeros[4] = {0.0, 0.0, 0.0, 0.0};
		glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, zeros);
	}

	glClearColor(0, 0, 0, 1);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glLineWidth(2.0);

	for (int i = 0; i < 100*100*100; i++) {
		image[i] = ((float)rand()) / RAND_MAX;
		//printf("%f\n", image[i]);
	}
	//image[50][99][0] = 1;
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, 100, 100, 100, GL_RED, GL_FLOAT, image);

	// Main loop
	int zoom_level = 0;
	float zoom_incr = 0.007;
	float shift[3];
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		// Events
		glfwPollEvents();
		//glfwWaitEvents();
		// Input
		char mouse_left_button	= glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)	== GLFW_PRESS;
		char zoom_in_key		= glfwGetKey(window, GLFW_KEY_EQUAL)					== GLFW_PRESS;
		char zoom_out_key		= glfwGetKey(window, GLFW_KEY_MINUS)					== GLFW_PRESS;
		char move_up_key		= glfwGetKey(window, GLFW_KEY_UP)						== GLFW_PRESS;
		char move_down_key		= glfwGetKey(window, GLFW_KEY_DOWN)						== GLFW_PRESS;
		char move_right_key		= glfwGetKey(window, GLFW_KEY_RIGHT)					== GLFW_PRESS;
		char move_left_key 		= glfwGetKey(window, GLFW_KEY_LEFT)						== GLFW_PRESS;
		// React to events
		if (
			mouse_left_button ||
			zoom_in_key || zoom_out_key	 ||
			move_up_key || move_down_key || move_right_key || move_left_key
		) {
			// Mouse position and texture coordinates
			float mouse_window[2];
			{
				double x, y;
				glfwGetCursorPos(window, &x, &y);
				// Convert to GL coordinates
				mouse_window[0] = 2.0 * (x - window_offset_x) / window_length - 1.0;
				mouse_window[1] = 1.0 - 2.0 * (y - window_offset_y) / window_length;
			}
			//printf("%f, %f\n", x, y);
			char plane = active_plane(mouse_window[0], mouse_window[1]);
			// Execute, dependent on which plane is active
			char update = 1;
			float mouse_tex_coord[2];
			if (plane == 0) { // (0, 1, 2)
				// Scale according to current zoomed view
				float mouse_tex_coord[2];
				plane_0_tex_coords(mouse_tex_coord, mouse_window, tex_coords[0][0], tex_coords[0][2]);
				if (mouse_left_button) {
					centres[0][0] = mouse_window[0];
					centres[0][1] = mouse_window[1];
					centres[1][0] = mouse_window[0];
					centres[2][1] = mouse_window[1];
					for (int i = 0; i < 4; i++) {
						tex_coords[2][i][0] = mouse_tex_coord[0];
						tex_coords[1][i][1] = mouse_tex_coord[1];
					}
				}
				if (zoom_in_key || zoom_out_key) {
					// Change zoom
					float zoom = 1;
					if (zoom_in_key)		{zoom -= zoom_incr; zoom_level -= 1;}
					else if (zoom_out_key)	{zoom += zoom_incr; zoom_level += 1;}
					// Change position of crosses
					plane_0_tex_coords(centres[0], centres[0], tex_coords[0][0], tex_coords[0][2]);
					// Zoom into texture
					float shift[2] = {0};
					for (int i = 0; i < 4; i++) {
						for (int j = 0; j < 2; j++) {
							float c = zoom * (tex_coords[0][i][j] - mouse_tex_coord[1-j]) + mouse_tex_coord[1-j];
							tex_coords[0][i][j] = c;
							if (c < 0)		c = -c;
							else if (c > 1)	c = 1 - c;
							else			c = 0;
							if (c != 0) shift[j] = c;
						}
					}
					for (int i = 0; i < 4; i++) {
						for (int j = 0; j < 2; j++) {
							tex_coords[0][i][j] = fmax(0, fmin(1, tex_coords[0][i][j] + shift[j]));
							// Still need fmax/fmin because of inaccuracy
						}
					}
					plane_0_window_coords(centres[0], centres[0], tex_coords[0][0], tex_coords[0][2]);
					// TODO: store cross centres (in texture coords) as separate variable to avoid numerical error if heavy zooming
				}
				if (move_up_key) {
					float zoom = 0.01 * powf((1 + zoom_incr), zoom_level / 100.0);
					printf("%f %f %d\n", zoom, 1 / zoom_incr, zoom_level);
					float new_coord[3];
					char is_outside = 0;
					for (int j = 0; j < 4; j++) {
						float c = tex_coords[0][j][0] + zoom;
						if (c < 0.0 || c > 1.0) {
							is_outside = 1;
							break;
						}
					}
					if (!is_outside) {
						for (int j = 0; j < 4; j++) {
							tex_coords[0][j][0] = new_coord[j];
							// = fmin(1, fmax(0, ));
							//tex_coords[2][j][0] = fmin(1, fmax(0, tex_coords[2][j][0] + zoom));
						}
					}
				}
			}
			else if (plane == 1) { // (0, 2, 1)
				if (mouse_left_button) {
					centres[1][0] = mouse_window[0];
					centres[1][1] = mouse_window[1];
					centres[0][0] = mouse_window[0];
					centres[2][0] = mouse_window[1] + 1.0;
					for (int i = 0; i < 4; i++) {
						tex_coords[0][i][2] = (mouse_window[1] - 0.02 + 1.0) / 0.96;
						tex_coords[2][i][0] = (mouse_window[0] - 0.02 + 1.0) / 0.96;
					}
				}
			}
			else if (plane == 2) { // (1, 2, 0)
				if (mouse_left_button) {
					centres[2][0] = mouse_window[0];
					centres[2][1] = mouse_window[1];
					centres[0][1] = mouse_window[1];
					centres[1][1] = mouse_window[0] - 1.0;
					for (int i = 0; i < 4; i++) {
						tex_coords[0][i][2] = (mouse_window[0] - 0.02) / 0.96;
						tex_coords[1][i][1] = (mouse_window[1] - 0.02) / 0.96;
					}
				}
			}
			else update = 0;
			if (update) {
				// TODO don't copy all?
				glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
				glBufferSubData(GL_ARRAY_BUFFER, sizeof(views), sizeof(tex_coords), tex_coords);
				glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
				glBufferSubData(GL_ARRAY_BUFFER, sizeof(crosses), sizeof(centres), centres);
			}
		}
		// Esc
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, 1);
		// Views
		glUseProgram(views_program);
		glBindVertexArray(views_vertex_array);
		for (int i = 0; i <= 8; i += 4) glDrawArrays(GL_TRIANGLE_FAN, i, 4);
		glUseProgram(crosses_program);
		glBindVertexArray(crosses_vertex_array);
		glUniform1i(cross_vertical, 0); 
		glDrawArraysInstanced(GL_LINES, 0, 2, 3);
		glUniform1i(cross_vertical, 1); 
		glDrawArraysInstanced(GL_LINES, 2, 2, 3);
		glFlush();
		glfwSwapBuffers(window);
	}

	// Clean up
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

