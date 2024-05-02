GLuint setup_texture(float *image, int size[3])
{
	GLuint texture;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_3D, texture);
	glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32F, size[0], size[1], size[2]);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	long rst_coordinates[3] = {GL_TEXTURE_WRAP_R, GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T};
	for (int i = 0; i < 3; i++) glTexParameteri(GL_TEXTURE_3D, rst_coordinates[i], GL_CLAMP_TO_BORDER);
	const float zeros[4] = {0.0, 0.0, 0.0, 0.0};
	glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, zeros);

	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, size[0], size[1], size[2], GL_RED, GL_FLOAT, image);

	return texture;
}

