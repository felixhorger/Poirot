
const float plane_tex_shift[3][2] = {
	{-0.02 + 1.0, -0.02      }, // (0, 1, 2)
	{-0.02 + 1.0, -0.02 + 1.0}, // (0, 2, 1)
	{-0.02      , -0.02      }  // (2, 1, 0)
};



void window2tex(char plane, float window_coord[2], float tex_coord[2], float tex_lower[3], float tex_upper[3]) {
	if (plane < 0 || plane > 2) {
		printf("Error: invalid plane %d", plane);
		exit(1);
	}
	for (int i = 0; i < 2; i++) {
		int axis = plane_axes[plane][i];
		tex_coord[i] = (window_coord[i] + plane_tex_shift[plane][i]) / 0.96 * (tex_upper[axis] - tex_lower[axis]) + tex_lower[axis];
	}
	return;
}



char current_plane(double x, double y)
{
	int plane = -1;
	if (x >= -0.98 && x <= -0.02) {
		if (y >= 0.02 && y <= 0.98)		plane = 0; // (0, 1, 2)
		else if (y <= -0.02 && y >= -0.98)	plane = 1; // (0, 2, 1)
	}
	else if (x >= 0.02 && x <= 0.98 && y >= 0.02 && y <= 0.98) plane = 2; // (2, 1, 0)
	return plane;
}



void tex2window(char plane, float tex_coord[2], float window_coord[2], float tex_lower[3], float tex_upper[3]) {
	if (plane < 0 || plane > 2) {
		printf("Error: invalid plane %d", plane);
		exit(1);
	}
	for (int i = 0; i < 2; i++) {
		int axis = plane_axes[plane][i];
		window_coord[i] = (tex_coord[i] - tex_lower[axis]) * 0.96 / (tex_upper[axis] - tex_lower[axis]) - plane_tex_shift[plane][i];
	}
	//printf("%f %f\n", window_coord[0], window_coord[1]);
	return;
}



// Changes only the first two elements in centers[:][0:1]
void update_cross_centres(char plane, float centres[3][2]) {
	switch (plane) {
		case 0:
			centres[1][0] = centres[0][0];
			centres[2][1] = centres[0][1];
			break;
		case 1:
			centres[0][0] = centres[1][0];
			centres[2][0] = centres[1][1];
			break;
		case 2:
			centres[0][1] = centres[2][1];
			centres[1][1] = centres[2][0];
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
		int slice = plane_axes[other_plane][2];
		for (int i = 0; i < 4; i++) {
			tex_coords[other_plane][i][slice] = tex_centres[plane][slice_axis[plane][other_plane]];
		}
	}
	return;
}



void move_view(float rel_shift[2], char p, float plane[4][3], float centre[4], float centre_window[2]) {
	for (int a = 0; a < 2; a++) {
		int axis = plane_axes[p][a];
		float move = rel_shift[a] * (plane[2][axis] - plane[0][axis]);
		for (int i = 0; i < 4; i++) {
			float c = plane[i][axis] + move;
			if (c < 0.0) move -= c;
			else if (c > 1.0) move += 1 - c;
		}
		for (int i = 0; i < 4; i++)
			plane[i][axis] = fmax(0, fmin(1, move + plane[i][axis]));
	}
	// Update position of cross in window
	tex2window(p, centre, centre_window, plane[0], plane[2]);
	return;
}

