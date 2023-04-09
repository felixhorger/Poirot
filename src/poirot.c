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

int window_width, window_height;
float ratio = 1;
int ratio_axis = 0;
char window_size_update = 0;
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	window_width = width;
	window_height = height;
	glViewport(0, 0, window_width, window_height);
	ratio = ((float)window_width) / ((float)window_height);
	if (ratio > 1) ratio_axis = 0;
	else {
		ratio_axis = 1;
		ratio = 1 / ratio;
	}
	window_size_update = 1;
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
	else if (x >= 0.02 && x <= 0.98 && y >= 0.02 && y <= 0.98) plane = 2; // (2, 1, 0)
	return plane;
}

// Views axes in arrays
const char views_axes[3][3] = {
	{0, 1, 2},
	{0, 2, 1},
	{2, 1, 0}
};

const float plane_tex_shift[3][2] = {
	{-0.02 + 1.0, -0.02			}, // (0, 1, 2)
	{-0.02 + 1.0, -0.02 + 1.0	}, // (0, 2, 1)
	{-0.02		, -0.02			}  // (2, 1, 0)
};

void compute_tex_coords(char plane, float tex_coord[2], float window_coord[2], float tex_lower[3], float tex_upper[3]) {
	if (plane < 0 || plane > 2) {
		printf("Error: invalid plane %d", plane);
		exit(1);
	}
	for (int i = 0; i < 2; i++) {
		int axis = views_axes[plane][i];
		tex_coord[i] = (window_coord[i] + plane_tex_shift[plane][i]) / 0.96 * (tex_upper[axis] - tex_lower[axis]) + tex_lower[axis];
	}
	return;
}
void compute_window_coords(char plane, float window_coord[2], float tex_coord[2], float tex_lower[3], float tex_upper[3]) {
	if (plane < 0 || plane > 2) {
		printf("Error: invalid plane %d", plane);
		exit(1);
	}
	for (int i = 0; i < 2; i++) {
		int axis = views_axes[plane][i];
		window_coord[i] = (tex_coord[i] - tex_lower[axis]) * 0.96 / (tex_upper[axis] - tex_lower[axis]) - plane_tex_shift[plane][i];
	}
	return;
}

// Changes only the first two elements in centers[:][0:1]
void update_cross_centres(char plane, float tex_centres[3][2]) {
	switch (plane) {
		case 0:
			tex_centres[1][0] = tex_centres[0][0];
			tex_centres[2][1] = tex_centres[0][1];
			break;
		case 1:
			tex_centres[0][0] = tex_centres[1][0];
			tex_centres[2][0] = tex_centres[1][1];
			break;
		case 2:
			tex_centres[0][1] = tex_centres[2][1];
			tex_centres[1][1] = tex_centres[2][0];
			break;
		default:
			printf("Error: invalid plane %d", plane);
			exit(1);
	}
	return;
}


void update_view_slices(char plane, float tex_coords[3][4][3], float tex_centres[3][2]) {
	if (plane < 0 || plane > 2) {
		printf("Error: invalid plane %d", plane);
		exit(1);
	}
	const char slice_axis[3][3] = {
		{2, 1, 0},
		{1, 2, 0},
		{0, 1, 2}
	};
	char other_planes[2] = {(plane + 2) % 3, (plane + 1) % 3};
	for (int p = 0; p < 2; p++) {
		char other_plane = other_planes[p];
		int slice = views_axes[other_plane][2];
		for (int i = 0; i < 4; i++) {
			tex_coords[other_plane][i][slice] = tex_centres[plane][slice_axis[plane][other_plane]];
		}
	}
	return;
}

void move_view(char plane, float rel_shift[2], float tex_coords[4][3], float centre[4], float tex_centre[2]) {
	for (int a = 0; a < 2; a++) {
		int axis = views_axes[plane][a];
		float move = rel_shift[a] * (tex_coords[2][axis] - tex_coords[0][axis]);
		for (int i = 0; i < 4; i++) {
			float c = tex_coords[i][axis] + move;
			if (c < 0.0) move -= c;
			else if (c > 1.0) move += 1 - c;
		}
		for (int i = 0; i < 4; i++)
			tex_coords[i][axis] = fmax(0, fmin(1, move + tex_coords[i][axis]));
	}
	// Update position of cross in window
	compute_window_coords(plane, centre, tex_centre, tex_coords[0], tex_coords[2]);
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
	window_width = 960;
	window_height = 960;
	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "", NULL, NULL); //glfwGetPrimaryMonitor()
	if (!window) {
		printf("Error: could not open window");
		return 1;
	}
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0); // activating this doesn't work well with wslg, fffff
	gladLoadGL();
	glViewport(0, 0, window_width, window_height);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(gl_error_callback, 0);

	// Views shaders
	GLuint views_program, views_shaders[2];
	{
		const char *shader_code[] = {
			"\
				#version 450 core										\n\
				layout (location = 0) in vec2 position;					\n\
				layout (location = 1) in vec3 in_tex_coordinate;		\n\
				uniform float ratio;									\n\
				uniform int axis;										\n\
				out vec3 tex_coordinate;								\n\
				void main() {											\n\
					gl_Position = vec4(position, 0.0, 1.0);				\n\
					tex_coordinate = in_tex_coordinate;					\n\
					tex_coordinate[axis] = (							\n\
						(tex_coordinate[axis] - 0.5) * ratio + 0.5		\n\
					);													\n\
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
	float tex_coords[3][4][3] = { // plane, corner, axis
		{ // (0, 1, 2)
			{ 0.0f, 0.0f, 0.5f },
			{ 1.0f, 0.0f, 0.5f },
			{ 1.0f, 1.0f, 0.5f },
			{ 0.0f, 1.0f, 0.5f }
		},
		{ // (0, 2, 1)
			{ 0.0f, 0.5f, 0.0f },
			{ 1.0f, 0.5f, 0.0f },
			{ 1.0f, 0.5f, 1.0f },
			{ 0.0f, 0.5f, 1.0f }
		},
		{ // (2, 1, 0)
			{ 0.5f, 0.0f, 0.0f },
			{ 0.5f, 0.0f, 1.0f },
			{ 0.5f, 1.0f, 1.0f },
			{ 0.5f, 1.0f, 0.0f }
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
		{ 0.5f,  0.5f,  0.5f,  0.5f}  // (2, 1, 0)
	};
	float tex_centres[3][2] = {
		{0.5f, 0.5f}, // (0, 1, 2)
		{0.5f, 0.5f}, // (0, 2, 1)
		{0.5f, 0.5f}  // (2, 1, 0)
	};

	GLuint buffers[2];
	glGenBuffers(2, &buffers[0]);
	// View
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(views)+sizeof(tex_coords), NULL, GL_DYNAMIC_STORAGE_BIT);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(views), views);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(views), sizeof(tex_coords), tex_coords);
	GLint uniform_ratio = glGetUniformLocation(views_program, "ratio");
	GLint uniform_ratio_axis = glGetUniformLocation(views_program, "axis");
	glUseProgram(views_program);
	glUniform1f(uniform_ratio, ratio);
	glUniform1i(uniform_ratio_axis, ratio_axis);
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

	float image[100][100][100] = { 0 };
	//for (int i = 0; i < 100*100*100; i++) {
	//	image[i] = ((float)rand()) / RAND_MAX;
	//	//printf("%f\n", image[i]);
	//}
	//for (int i = 0; i < 100; i++) {
	//	for (int j = 0; j < 100; j++) {
	//		for (int k = 0; k < 100; k++) {
	//			image[i][j][k] = (i * j * k) / 1000000.0;
	//		}
	//	}
	//}
	for (int i = 49; i < 52; i++) {
		for (int j = 49; j < 52; j++) {
			for (int k = 49; k < 52; k++) {
				image[i][j][k] = 1;
			}
		}
	}
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, 100, 100, 100, GL_RED, GL_FLOAT, image);

	// Main loop
	float zoom_incr = 0.0075;
	float move_speed = 0.0075;
	float mouse_window[2];
	float mouse_delta[2];
	char was_plane = -1;
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		// Events
		glfwPollEvents(); //glfwWaitEvents();
		// Mouse position
		{
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			// Convert to GL coordinates
			x = 2.0 * x / window_width - 1.0;
			y = 1.0 - 2.0 * y / window_height;
			//printf("%f, %f\n", x, y);
			mouse_delta[0] = mouse_window[0] - x;
			mouse_delta[1] = mouse_window[1] - y;
			mouse_window[0] = x;
			mouse_window[1] = y;
		}
		// Input
		char mouse_right_button_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
		char mouse_right_button	= mouse_right_button_state == GLFW_PRESS;
		char mouse_left_button	= glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)	== GLFW_PRESS;
		char zoom_in_key		= glfwGetKey(window, GLFW_KEY_EQUAL)					== GLFW_PRESS;
		char zoom_out_key		= glfwGetKey(window, GLFW_KEY_MINUS)					== GLFW_PRESS;
		char move_up_key		= glfwGetKey(window, GLFW_KEY_UP)						== GLFW_PRESS;
		char move_down_key		= glfwGetKey(window, GLFW_KEY_DOWN)						== GLFW_PRESS;
		char move_right_key		= glfwGetKey(window, GLFW_KEY_RIGHT)					== GLFW_PRESS;
		char move_left_key 		= glfwGetKey(window, GLFW_KEY_LEFT)						== GLFW_PRESS;
		// React to events
		if (mouse_right_button_state == GLFW_RELEASE) was_plane = -1;
		if (
			mouse_left_button || mouse_right_button ||
			zoom_in_key || zoom_out_key  ||
			move_up_key || move_down_key || move_right_key || move_left_key
		) {
			char plane = active_plane(mouse_window[0], mouse_window[1]);
			// Execute, dependent on which plane is active
			char update = 1;
			float mouse_tex_coord[2];

			if (plane != -1) {
				compute_tex_coords(plane, mouse_tex_coord, mouse_window, tex_coords[plane][0], tex_coords[plane][2]);
				if (mouse_left_button) {
					// Change position of cross of plane
					for (int i = 0; i < 2; i++) centres[plane][i] = mouse_window[i];
					// Compute tex coordinate from that
					compute_tex_coords(plane, tex_centres[plane], centres[plane], tex_coords[plane][0], tex_coords[plane][2]);
					// Copy tex coordinates into crosses of other planes
					update_cross_centres(plane, tex_centres);
					// Update window coordinates of crosses
					for (int i = 0; i < 3; i++) {
						if (i == plane) continue;
						compute_window_coords(i, centres[i], tex_centres[i], tex_coords[i][0], tex_coords[i][2]);
					}
					// Change slices in other views
					update_view_slices(plane, tex_coords, tex_centres);
				}
				if (zoom_in_key || zoom_out_key) {
					// Change zoom
					float zoom = 1;
					if (zoom_in_key)		zoom -= zoom_incr;
					else if (zoom_out_key)	zoom += zoom_incr;
					// Zoom into texture
					float shift[2] = {0};
					for (int i = 0; i < 4; i++) {
						for (int j = 0; j < 2; j++) {
							int axis = views_axes[plane][j];
							float c = zoom * (tex_coords[plane][i][axis] - mouse_tex_coord[j]) + mouse_tex_coord[j];
							tex_coords[plane][i][axis] = c;
							if (c < 0)		c = -c;
							else if (c > 1)	c = 1 - c;
							else			c = 0;
							if (c != 0) shift[j] = c;
						}
					}
					for (int i = 0; i < 4; i++) {
						for (int j = 0; j < 2; j++) {
							int axis = views_axes[plane][j];
							tex_coords[plane][i][axis] = fmax(0, fmin(1, tex_coords[plane][i][axis] + shift[j])); // Still need fmax/fmin because of inaccuracy
						}
					}
					// Update position of cross in window
					compute_window_coords(plane, centres[plane], tex_centres[plane], tex_coords[plane][0], tex_coords[plane][2]);
				}
				if (move_up_key || move_down_key || move_right_key || move_left_key) {
					char sign = move_up_key || move_right_key ? 1 : -1;
					char axis = move_up_key || move_down_key ? 1 : 0;
					float rel_shift[2] = {0};
					rel_shift[axis] = sign * move_speed;
					move_view(plane, rel_shift, tex_coords[plane], centres[plane], tex_centres[plane]);
				}
				if (was_plane == -1) was_plane = plane;
			}

			if (was_plane != -1) {
				if (mouse_right_button) {
					float rel_shift[2];
					for (int i = 0; i < 2; i++) rel_shift[i] = 100 * mouse_delta[i] * move_speed;
					move_view(was_plane, rel_shift, tex_coords[was_plane], centres[was_plane], tex_centres[was_plane]);
				}
			}

			if (plane == 0 || was_plane == 0) { // (0, 1, 2)
				// Scale according to current zoomed view
				//if (zoom_in_key || zoom_out_key) {
				//	// Change zoom
				//	float zoom = 1;
				//	if (zoom_in_key)		zoom -= zoom_incr;
				//	else if (zoom_out_key)	zoom += zoom_incr;
				//	// Zoom into texture
				//	float shift[2] = {0};
				//	for (int i = 0; i < 4; i++) {
				//		for (int j = 0; j < 2; j++) {
				//			float c = zoom * (tex_coords[0][i][j] - mouse_tex_coord[1-j]) + mouse_tex_coord[1-j];
				//			tex_coords[0][i][j] = c;
				//			if (c < 0)		c = -c;
				//			else if (c > 1)	c = 1 - c;
				//			else			c = 0;
				//			if (c != 0) shift[j] = c;
				//		}
				//	}
				//	for (int i = 0; i < 4; i++) {
				//		for (int j = 0; j < 2; j++) {
				//			tex_coords[0][i][j] = fmax(0, fmin(1, tex_coords[0][i][j] + shift[j]));
				//			// Still need fmax/fmin because of inaccuracy
				//		}
				//	}
				//	// Update position of cross in window
				//	compute_window_coords(plane, centres[0], tex_centres[0], tex_coords[0][0], tex_coords[0][2]);
				//}
				//if (mouse_right_button) {
				//	was_plane = 0;
				//	float move;
				//	for (int axis = 0; axis < 2; axis++) {
				//		move = 100 * mouse_delta[1-axis] * move_speed * (tex_coords[0][2][axis] - tex_coords[0][0][axis]);
				//		for (int i = 0; i < 4; i++) {
				//			float c = tex_coords[0][i][axis] + move;
				//			if (c < 0.0) move -= c;
				//			else if (c > 1.0) move += 1 - c;
				//		}
				//		for (int i = 0; i < 4; i++)
				//			tex_coords[0][i][axis] = fmax(0, fmin(1, move + tex_coords[0][i][axis]));
				//	}
				//}
				//if (move_up_key || move_down_key || move_right_key || move_left_key) {
				//	char sign = move_up_key || move_right_key ? 1 : -1;
				//	char axis = move_up_key || move_down_key ? 0 : 1;
				//	float move = sign * move_speed * (tex_coords[0][2][axis] - tex_coords[0][0][axis]);
				//	for (int i = 0; i < 4; i++) {
				//		float c = tex_coords[0][i][axis] + move;
				//		if (c < 0.0) move -= c;
				//		else if (c > 1.0) move += 1 - c;
				//	}
				//	for (int i = 0; i < 4; i++)
				//		tex_coords[0][i][axis] = fmax(0, fmin(1, move + tex_coords[0][i][axis]));
				//}
			}
			//else if (plane == 1) { // (0, 2, 1)
			//	if (mouse_left_button) {
			//		centres[1][0] = mouse_window[0];
			//		centres[1][1] = mouse_window[1];
			//		centres[0][0] = mouse_window[0];
			//		centres[2][0] = mouse_window[1] + 1.0;
			//		for (int i = 0; i < 4; i++) {
			//			tex_coords[0][i][2] = (mouse_window[1] - 0.02 + 1.0) / 0.96;
			//			tex_coords[2][i][0] = (mouse_window[0] - 0.02 + 1.0) / 0.96;
			//		}
			//	}
			//}
			//else if (plane == 2) { // (2, 1, 0)
			//	if (mouse_left_button) {
			//		centres[2][0] = mouse_window[0];
			//		centres[2][1] = mouse_window[1];
			//		centres[0][1] = mouse_window[1];
			//		centres[1][1] = mouse_window[0] - 1.0;
			//		for (int i = 0; i < 4; i++) {
			//			tex_coords[0][i][2] = (mouse_window[0] - 0.02) / 0.96;
			//			tex_coords[1][i][1] = (mouse_window[1] - 0.02) / 0.96;
			//		}
			//	}
			//}
			//else update = 0; // TODO this is useless, make it depend on keys pressed
			if (update) {
				// TODO don't copy all?
				glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
				glBufferSubData(GL_ARRAY_BUFFER, sizeof(views), sizeof(tex_coords), tex_coords);
				glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
				glBufferSubData(GL_ARRAY_BUFFER, sizeof(crosses), sizeof(centres), centres);
			}
		}
		// Escape
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) break; //glfwSetWindowShouldClose(window, 1);
		// Views
		glUseProgram(views_program);
		glBindVertexArray(views_vertex_array);
		if (window_size_update) {
			glUniform1f(uniform_ratio, ratio);
			window_size_update = 0;
		}
		for (int i = 0; i <= 8; i += 4) {
			glUniform1i(uniform_ratio_axis, views_axes[i / 4][ratio_axis]);
			glDrawArrays(GL_TRIANGLE_FAN, i, 4);
		}
		// Crosses
		glUseProgram(crosses_program);
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
	glfwTerminate();

	return 0;
}

