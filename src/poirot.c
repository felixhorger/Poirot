
#include <stdlib.h>
#include <stdio.h>
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

char mouse_button_left = 0;
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) mouse_button_left = 1;
	else mouse_button_left = 0;
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
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwMakeContextCurrent(window);
	gladLoadGL();
	glViewport(0, 0, window_length, window_length);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(gl_error_callback, 0);

	// Views shaders
	GLuint views_program, views_shaders[2];
	{
		const char *shader_code[] = {
			"\
				#version 450 core										\n\
				layout (location = 0) in vec2 in_position;				\n\
				layout (location = 1) in vec3 in_tex_coordinate;		\n\
				out vec3 tex_coordinate;								\n\
				void main() {											\n\
					gl_Position = vec4(in_position, 0.0, 1.0);			\n\
					tex_coordinate = in_tex_coordinate;	\n\
				}														\n\
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
	float tex_coords[3][4][3] = {
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
				layout (location = 1) in vec2 centre;			\n\
				layout (location = 2) in vec4 in_limits;			\n\
				out vec2 position;					\n\
				out vec4 limits;					\n\
				void main() {									\n\
					gl_Position = vec4(in_position + centre, 0.0, 1.0);		\n\
					position = gl_Position.xy; \n\
					limits = in_limits;							\n\
				}												\n\
			",
			"\
				#version 450 core							\n\
				in vec2 position;					\n\
				in vec4 limits;					\n\
				out vec4 colour; \n\
				void main(void) {							\n\
					if (									\n\
						position[0] >= limits[0] &&   \n\
						position[0] <= limits[1] &&   \n\
						position[1] >= limits[2] &&   \n\
						position[1] <= limits[3]   \n\
					)		colour = vec4(0.0, 1.0, 0.0, 1.0);		\n\
					else	discard;		\n\
				}											\n\
			"
		};
		create_program(shader_code, &crosses_program, crosses_shaders);
	}
	const float crosses[] = {
		-0.96f,  0.00f,
		 0.96f,  0.00f,
		 0.00f, -0.96f,
		 0.00f,  0.96f
	};
	float centres[3][2] = {
		-0.5f,  0.5f,
		-0.5f, -0.5f,
		 0.5f,  0.5f
	};
	const float limits[] = {
		-0.98f, -0.02f,  0.02f,  0.98f, // (0, 1, 2)
		-0.98f, -0.02f, -0.98f, -0.02f, // (0, 2, 1)
		 0.02f,  0.98f,  0.02f,  0.98f  // (1, 2, 0)
	};

	float image[100*100*100] = {1};

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
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(crosses)+sizeof(centres)+sizeof(limits), NULL, GL_DYNAMIC_STORAGE_BIT);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(crosses), crosses);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(crosses), sizeof(centres), centres);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(crosses)+sizeof(centres), sizeof(limits), limits);
	// 
	GLuint crosses_vertex_array;
	glGenVertexArrays(1, &crosses_vertex_array);
	glBindVertexArray(crosses_vertex_array);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)(sizeof(crosses)));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void *)(sizeof(crosses)+sizeof(centres)));
	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	// Texture
	GLuint texture;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_3D, texture);
	glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32F, 100, 100, 100);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


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
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, 100, 100, 100, GL_RED, GL_FLOAT, image);

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		// Input
		if (mouse_button_left) {
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			x = 2.0 * (x - window_offset_x) / window_length  - 1.0;
			y = 1.0 - 2.0 * (y - window_offset_y) / window_length;
			char update = 1;
			//printf("%f, %f\n", x, y);
			if (x >= -0.98 && x <= -0.02) {
				if (y >= 0.02 && y <= 0.98) { // (0, 1, 2)
					centres[0][0] = x;
					centres[0][1] = y;
					centres[1][0] = x;
					centres[2][1] = y;
					for (int i = 0; i < 4; i++) {
						tex_coords[1][i][1] = (y - 0.02) / 0.96;
						tex_coords[2][i][0] = (x - 0.02 + 1.0) / 0.96;
					}
				}
				else if (y <= -0.02 && y >= -0.98) { // (0, 2, 1)
					centres[1][0] = x;
					centres[1][1] = y;
					centres[0][0] = x;
					centres[2][0] = y + 1.0;
					for (int i = 0; i < 4; i++) {
						tex_coords[0][i][2] = (y - 0.02 + 1.0) / 0.96;
						tex_coords[2][i][0] = (x - 0.02 + 1.0) / 0.96;
					}
				}
				else update = 0;
			}
			else if (x >= 0.02 && x <= 0.98 && y >= 0.02 && y <= 0.98) { // (0, 2, 1)
				centres[2][0] = x;
				centres[2][1] = y;
				centres[0][1] = y;
				centres[1][1] = x - 1.0;
				for (int i = 0; i < 4; i++) {
					tex_coords[0][i][2] = (x - 0.02) / 0.96;
					tex_coords[1][i][1] = (y - 0.02) / 0.96;
				}
			}
			else update = 0;
			if (update) {
				// TODO don't copy all
				glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
				glBufferSubData(GL_ARRAY_BUFFER, sizeof(views), sizeof(tex_coords), tex_coords);
				glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
				glBufferSubData(GL_ARRAY_BUFFER, sizeof(crosses), sizeof(centres), centres);
			}
		}
		// Views
		glUseProgram(views_program);
		glBindVertexArray(views_vertex_array);
		for (int i = 0; i <= 8; i += 4) glDrawArrays(GL_TRIANGLE_FAN, i, 4);
		glUseProgram(crosses_program);
		glBindVertexArray(crosses_vertex_array);
		glDrawArraysInstanced(GL_LINES, 0, 2, 3);
		glDrawArraysInstanced(GL_LINES, 2, 2, 3);
		glFlush();
		glfwSwapBuffers(window);
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, 1);
	}

	// Clean up
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

