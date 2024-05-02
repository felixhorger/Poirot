
const float plane_vertices[] = {
	-0.98f, -0.98f,
	 0.98f,  0.98f,
	 0.98f,  0.98f,
	 0.98f,  0.98f
};



const char plane_axes[3][3] = {
	{0, 1, 2},
	{0, 2, 1},
	{2, 1, 0}
};

const float three_planes_vertices[] = {
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



const float crosses[] = {
	-0.48f,  0.00f,
	 0.48f,  0.00f,
	 0.00f, -0.48f,
	 0.00f,  0.48f
};



GLuint setup_three_planes(GLuint buffer, float planes[3][4][3])
{
	size_t sizeof_planes = sizeof(float) * 3 * 4 * 3;
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferStorage(
		GL_ARRAY_BUFFER,
		sizeof(three_planes_vertices) + sizeof_planes,
		NULL,
		GL_DYNAMIC_STORAGE_BIT
	);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(three_planes_vertices), three_planes_vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(three_planes_vertices), sizeof_planes, planes);

	GLuint three_planes_vertex_array;
	glGenVertexArrays(1, &three_planes_vertex_array);
	glBindVertexArray(three_planes_vertex_array);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)sizeof(three_planes_vertices));

	return three_planes_vertex_array;
}


GLuint setup_crosses(GLuint buffer, float centres[3][4]) // belongs here?
{
	size_t sizeof_centres = sizeof(float) * 3 * 4;
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(crosses) + sizeof_centres, NULL, GL_DYNAMIC_STORAGE_BIT);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(crosses), crosses);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(crosses), sizeof_centres, centres);

	GLuint crosses_vertex_array;
	glGenVertexArrays(1, &crosses_vertex_array);
	glBindVertexArray(crosses_vertex_array);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void *)(sizeof(crosses)));
	glVertexAttribDivisor(1, 1);

	return crosses_vertex_array;
}

