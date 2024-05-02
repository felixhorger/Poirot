
void error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error: %s\n", description);
	exit(EXIT_FAILURE);
	return;
}



GLFWwindow* open_window(int width, int height)
{
	GLFWwindow* window = glfwCreateWindow(width, height, "", NULL, NULL); //glfwGetPrimaryMonitor()
	if (!window) {
		printf("Error: could not open window");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0); // TODO: turn on?

	return window;
}



void update_mouse_position(GLFWwindow *window, int width, int height, float mouse_window[2], float mouse_delta[2])
{
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	// Convert to GL coordinates
	x = 2.0 * x / width - 1.0;
	y = 1.0 - 2.0 * y / height;
	//printf("%f, %f\n", x, y);

	mouse_delta[0] = mouse_window[0] - x;
	mouse_delta[1] = mouse_window[1] - y;

	mouse_window[0] = x;
	mouse_window[1] = y;

	return;
}

