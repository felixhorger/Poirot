float planes[3][4][3] = { // plane, corner, axis
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

float centres[3][2] = {
	{0.5f, 0.5f}, // (0, 1, 2)
	{0.5f, 0.5f}, // (0, 2, 1)
	{0.5f, 0.5f}  // (2, 1, 0)
};



float centres_window[3][4] = {
	{-0.5f,  0.5f, -0.5f,  0.5f}, // (0, 1, 2)
	{-0.5f, -0.5f, -0.5f, -0.5f}, // (0, 2, 1)
	{ 0.5f,  0.5f,  0.5f,  0.5f}  // (2, 1, 0)
};



