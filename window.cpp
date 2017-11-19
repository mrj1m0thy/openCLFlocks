#include "window.h"

//Struct used to handle parallel for loop that executes the draw flock function
struct Window::tbbDrawFlocks {
	//A reference to the flock needs to be captured in order to maintain any changes
	std::vector<Flock>* tbbFlocks;

	tbbDrawFlocks(std::vector<Flock>* f) {
		tbbFlocks = f;
	}

	//This is where the parallel for takes place
	void operator()(const tbb::blocked_range<size_t>& range) const {
		for (size_t i = range.begin(); i < range.end(); i++) {
			tbbFlocks->at(i).drawFlock();
		}
	}
};

void Window::start()
{
	GLFWwindow *window;

	// Initialize the library
	if (!glfwInit())
	{
		return;
	}

	// Create a windowed mode window and its OpenGL context
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello World", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return;
	}

	// Make the window's context current
	glfwMakeContextCurrent(window);

	glViewport(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT); // specifies the part of the window to which OpenGL will draw (in pixels), convert from normalised to pixels
	glMatrixMode(GL_PROJECTION); // projection matrix defines the properties of the camera that views the objects in the world coordinate frame. Here you typically set the zoom factor, aspect ratio and the near and far clipping planes
	glLoadIdentity(); // replace the current matrix with the identity matrix and starts us a fresh because matrix transforms such as glOrpho and glRotate cumulate, basically puts us at (0, 0, 0)
	glOrtho(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT, 0, 1); // essentially set coordinate system
	glMatrixMode(GL_MODELVIEW); // (default matrix mode) modelview matrix defines how your objects are transformed (meaning translation, rotation and scaling) in your world
	glLoadIdentity(); // same as above comment

	float colors[NUM_FLOCKS][3];

	//Fill flocks vector with new flocks
	for (int i = 0; i < NUM_FLOCKS; i++) {
		flocks.push_back(Flock());
		for (int j = 0; j < 3; j++)
			colors[i][j] = ((float)rand() / (RAND_MAX));
	}

	glClearColor(0.0f, 0.0f, 0.5f, 0.0f);

	std::clock_t start = std::clock();
	double duration;
	// Loop until the user closes the window
	while (!glfwWindowShouldClose(window))
	{
		//Restrict the rendering to 30 fps
		do {
			duration = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
		} while (duration < 33.3);
		start = std::clock();

		glClear(GL_COLOR_BUFFER_BIT);

		//Initialize the struct by passing a reference to the flocks
		tbbDrawFlocks DF = tbbDrawFlocks(&flocks);
		//Start parallel for on the draw flock
		tbb::parallel_for(tbb::blocked_range<size_t>(0, flocks.size()), DF);

		//Keep any changes made by the draw flock function
		flocks = *DF.tbbFlocks;

		for (int i = 0; i < NUM_FLOCKS; i++) {
			glColor3f(colors[i][0], colors[i][1], colors[i][2]);
			// Draw the birds/circles
			for (int j = 0; j < flocks[i].NUM_BIRDS; j++) {
				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(3, GL_FLOAT, 0, flocks[i].vertices[j]);
				glDrawArrays(GL_TRIANGLE_FAN, 0, flocks[i].numberOfVertices);
				glDisableClientState(GL_VERTEX_ARRAY);
			}

			//Delete the vertices
			for (int j = 0; j < flocks[i].NUM_BIRDS; j++) {
				delete [] flocks[i].vertices[j];
			}
		}

		// Swap front and back buffers
		glfwSwapBuffers(window);

		// Poll for and process events
		glfwPollEvents();
		//system("pause");
	}

	glfwTerminate();
}