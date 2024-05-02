

void setup_plane_shaders(GLuint* program, GLuint shaders[2]) {
	const char *vertex_shader_source = "\
		#version 450 core                                                          \n\
		layout (location = 0) in vec2 position;                                    \n\
		layout (location = 1) in vec3 in_tex_coordinate;                           \n\
		uniform float ratio;                                                       \n\
		uniform int axis;                                                          \n\
		out vec3 tex_coordinate;                                                   \n\
		void main() {                                                              \n\
			gl_Position = vec4(position, 0.0, 1.0);                            \n\
			tex_coordinate = in_tex_coordinate;                                \n\
			tex_coordinate[axis] = (tex_coordinate[axis] - 0.5) * ratio + 0.5; \n\
		}                                                                          \n\
	";
	const char *fragment_shader_source = "\
		#version 450 core                              \n\
		uniform sampler3D tex;                         \n\
		in vec3 tex_coordinate;                        \n\
		out vec4 colour;                               \n\
		void main(void) {                              \n\
			colour = texture(tex, tex_coordinate); \n\
		}                                              \n\
	";
	shaders[0] = glMakeShader(GL_VERTEX_SHADER, &vertex_shader_source);
	shaders[1] = glMakeShader(GL_FRAGMENT_SHADER, &fragment_shader_source);
	*program = glMakeProgram(shaders, 2);
	return;
}



void setup_cross_shaders(GLuint* program, GLuint shaders[2]) {
	// TODO: make centre a uniform? can't remember but seems like centre[2:3] are limits, and only centre[0:1] are updated
	const char *vertex_shader_source = "\
		#version 450 core                                               \n\
		layout (location = 0) in vec2 in_position;                      \n\
		layout (location = 1) in vec4 centre;                           \n\
		uniform bool vertical;                                          \n\
		out vec2 position;                                              \n\
		out vec2 offset;                                                \n\
		void main() {                                                   \n\
			if (vertical) {                                         \n\
				if (abs(centre[0] - centre[2]) > 0.48) {        \n\
					gl_Position = vec4(2.0, 0.0, 0.0, 1.0); \n\
					return;                                 \n\
				}                                               \n\
				offset[0] = centre[0];                          \n\
				offset[1] = centre[3];                          \n\
			}                                                       \n\
			else {                                                  \n\
				if (abs(centre[1] - centre[3]) > 0.48) {        \n\
					gl_Position = vec4(2.0, 0.0, 0.0, 1.0); \n\
					return;                                 \n\
				}                                               \n\
				offset[0] = centre[2];                          \n\
				offset[1] = centre[1];                          \n\
			}                                                       \n\
			gl_Position = vec4(in_position + offset, 0.0, 1.0);     \n\
			position = gl_Position.xy;                              \n\
		}                                                               \n\
	";

	const char *fragment_shader_source = "\
		#version 450 core                                       \n\
		in vec2 position;                                       \n\
		in vec2 offset;                                         \n\
		out vec4 colour;                                        \n\
		void main(void) {                                       \n\
			if (                                            \n\
				abs(position[0] - offset[0]) <= 0.48 && \n\
				abs(position[1] - offset[1]) <= 0.48    \n\
			)	colour = vec4(0.0, 1.0, 0.0, 1.0);      \n\
			else discard;                                   \n\
		}                                                       \n\
	";
	shaders[0] = glMakeShader(GL_VERTEX_SHADER, &vertex_shader_source);
	shaders[1] = glMakeShader(GL_FRAGMENT_SHADER, &fragment_shader_source);
	*program = glMakeProgram(shaders, 2);
	return;
}

