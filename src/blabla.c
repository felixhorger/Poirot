
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
