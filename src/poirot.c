
#include <stdlib.h>
#include <stdio.h>
#include "../include/glad.h"
#include <GLFW/glfw3.h>


void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: GLFW: %s\n", description);
	exit(1);
	return;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }

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
	GLFWwindow* window = glfwCreateWindow(800, 600, "", NULL, NULL); //glfwGetPrimaryMonitor()
	if (!window) {
		printf("Error: could not open window");
		return 1;
	}
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwMakeContextCurrent(window);
	gladLoadGL();
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	// Shaders
	{
		GLuint program = glCreateProgram();
		// Compile shaders and attach to program
		const char *shader_code[] = {
			"\
				#version 420 core									\n\
				layout (location = 0) in vec2 in_position;			\n\
				layout (location = 1) in vec2 in_tex_coordinate;	\n\
				out vec2 tex_coordinate;							\n\
				void main() {										\n\
					gl_Position = vec4(in_position, 0.0, 1.0);		\n\
					tex_coordinate = in_tex_coordinate;				\n\
				}													\n\
			",
			"\
				#version 420 core									\n\
				uniform sampler2D tex;								\n\
				//uniform int slice;								\n\
				in vec2 tex_coordinate;								\n\
				out vec4 colour;									\n\
				void main(void) {									\n\
					vec4 c = texture(tex, tex_coordinate);			\n\
					colour = c;										\n\
				}													\n\
			"
		};
		const GLenum shader_type[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
		for (int i = 0; i < 2; i++) {
			GLuint shader = glCreateShader(shader_type[i]);
			glShaderSource(shader, 1, &shader_code[i], NULL);
			glCompileShader(shader);
			check_program(shader, GL_COMPILE_STATUS, glGetShaderiv, glGetShaderInfoLog, glDeleteShader, "Could not compile shader");
			glAttachShader(program, shader);
		}
		// Finalise program
		glLinkProgram(program);
		check_program(program, GL_LINK_STATUS, glGetProgramiv, glGetProgramInfoLog, glDeleteProgram, "Could not link program");
		glUseProgram(program);
		glValidateProgram(program);
		check_program(program, GL_VALIDATE_STATUS, glGetProgramiv, glGetProgramInfoLog, glDeleteProgram, "Program invalid");
	}
	const float rectangle[] = {
		// Vertices
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f,
		-1.0f,  1.0f,
		// Texture coordinates
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};
	float image[100*100];
	for (int i = 0; i < 100*100; i++) {
		image[i] = ((float)rand()) / RAND_MAX;
		printf("%f\n", image[i]);
	}
	//
	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle), rectangle, GL_STATIC_DRAW);
	//
	GLuint vertex_array;;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)(8*sizeof(float)));
	GLuint texture;;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, 100, 100);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 100, 100, GL_RED, GL_FLOAT, image);
	//#slice = glUniform1i(glGetUniformLocation(program, "slice"), 0);
	glClearColor(0, 0, 0, 1);
	//#glEnable(GL_CULL_FACE)
	//#glCullFace(GL_BACK)
//	while !GLFW.WindowShouldClose(window)
//		glClear(GL_COLOR_BUFFER_BIT)
//		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//		glFlush()
//		GLFW.SwapBuffers(window)
//		GLFW.PollEvents()
//		if GLFW.GetKey(window, GLFW.KEY_ESCAPE) == GLFW.PRESS
//			GLFW.SetWindowShouldClose(window, true)
//		end
//	end


	// Main loop
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glFlush();
		glfwSwapBuffers(window);
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, 1);
	}

	// Clean up
	//glfwDestroyWindow(window);
	//glfwTerminate();

	return 0;
}

