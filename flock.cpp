#include "flock.h"

//Structure that handles the drawing of each individual bird
struct Flock::tbbDrawBirds {
	//Verticies and birds are needed to execute the function
	float** tbbVertices;
	Bird* tbbBirds;
	tbbDrawBirds(Bird* b, float** v) {
		tbbVertices = v;
		tbbBirds = b;
	}

	//Execute parallel for to draw birds
	void operator()(const tbb::blocked_range<size_t>& range) const {
		for (size_t i = range.begin(); i < range.end(); i++) {
			drawBird(tbbBirds[i], tbbVertices[i]);
		}
	}
};

//Struct that handles all the calculations to find the bird positions
//struct Flock::calculateBirdPositions {
//	//A reference to the birds is needed in order to update their positions and velocity
//	Bird* tbbBirds;
//
//	calculateBirdPositions(Bird*  b) {
//		tbbBirds = b;
//	}
//
//	//Execute parallel for loop to calculate the birds positions and velocities
//	void operator()(const tbb::blocked_range<size_t>& range) const {
//		for (size_t i = range.begin(); i < range.end(); i++) {
//			calculateBird(i, tbbBirds);
//		}
//	}
//};

//Initialize all the birds
Flock::Flock() {
	birds = new Bird[NUM_BIRDS];
	coordinateX = 250;
	coordinateY = 240;
	for (int i = 0; i < NUM_BIRDS; i++) {
		int height = rand() % SCREEN_HEIGHT;
		int width = rand() % SCREEN_WIDTH;
		if (width + (i * 25) >= SCREEN_WIDTH)
			width = -(i * 25);

		Bird b;
		boost::uuids::random_generator gen;
		const char* val = boost::uuids::to_string(gen()).c_str();
		b.uuid = new char[strlen(val)+1];
		strcpy_s(b.uuid, _TRUNCATE, val);
		b.velocity[0] = 0;
		b.velocity[1] = 0;
		b.position[0] = width + (i * 25);
		b.position[1] = height;

		birds[i] = b;
		//delete val;
	}
}

//Destroy all the vertices
Flock::~Flock() {
	//for (int i = 0; i < NUM_BIRDS; i++) {
	//	delete[] birds[i].position;
	//	delete[] birds[i].velocity;
	//	delete[] birds[i].uuid;
	//	//delete [] vertices[i];
	//}
	////delete [] vertices;
}

void Flock::drawFlock() {
	vertices = new float*[NUM_BIRDS];

	//Initialize placeholders for vertices
	for (int i = 0; i < NUM_BIRDS; i++) {
		vertices[i] = new float[numberOfVertices * 3];
	}

	loadKernel("vector_add_kernel.cl");
	getPlatInfo();

	// Create an OpenCL context
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
	printf("ret at %d is %d\n", __LINE__, ret);

	// Create a command queue
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
	printf("ret at %d is %d\n", __LINE__, ret);

	// Create memory buffers on the device for each vector 
	cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE,
		NUM_BIRDS * sizeof(Bird), NULL, &ret);

	/*char* message = new char[10000];
	cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
		10000 * sizeof(char), NULL, &ret);*/

	//NOTE: ret = return value or error value returned from the function
	// Copy the lists A and B to their respective memory buffers
	ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0,
		NUM_BIRDS * sizeof(Bird), birds, 0, NULL, NULL);
	printf("ret at %d is %d\n", __LINE__, ret);

	//ret = clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0,
	//	10000 * sizeof(char), message, 0, NULL, NULL);


	printf("before building\n");
	// Create a program from the kernel source
	cl_program program = clCreateProgramWithSource(context, 1,
		(const char **)&source_str, (const size_t *)&source_size, &ret);
	printf("ret at %d is %d\n", __LINE__, ret);

	// Build the program
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	printf("ret at %d is %d\n", __LINE__, ret);

	printf("after building\n");
	// Create the OpenCL kernel
	cl_kernel kernel = clCreateKernel(program, "calculateBird", &ret);
	printf("ret at %d is %d\n", __LINE__, ret);
	///////////////////////////////////////////////////////////////

	// The following is how the function parameters are passed to the function
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
	printf("ret at %d is %d\n", __LINE__, ret);

	printf("before execution\n");
	// Execute the OpenCL kernel on the list
	size_t global_item_size = NUM_BIRDS; // Process the entire lists
	size_t local_item_size = 4; // Divide work items into groups of 64
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
		&global_item_size, &local_item_size, 0, NULL, NULL);
	printf("after execution\n");
	// Read the memory buffer C on the device to the local variable C
	ret = clEnqueueReadBuffer(command_queue, a_mem_obj, CL_TRUE, 0,
		NUM_BIRDS * sizeof(Bird), birds, 0, NULL, NULL);
	printf("after copying\n");


	// Clean up
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(a_mem_obj);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);
	delete source_str;

	tbbDrawBirds DB = tbbDrawBirds(birds, vertices);
	tbb::parallel_for(tbb::blocked_range<size_t>(0, NUM_BIRDS), DB);

	//Set birds to what they were changed to
	//birds = DB.tbbBirds;
}

void Flock::loadKernel(char* name) {
	// Load the kernel source code into the array source_str
	fopen_s(&fp, name, "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);
	printf("kernel loading done\n");
}

void Flock::getPlatInfo() {
	//// Get platform and device information
	//ret = clGetPlatformIDs(0, NULL, &ret_num_platforms);
	//platforms = (cl_platform_id*)malloc(ret_num_platforms * sizeof(cl_platform_id));

	//ret = clGetPlatformIDs(ret_num_platforms, platforms, NULL);
	//printf("ret at %d is %d\n", __LINE__, ret);

	//ret = clGetDeviceIDs(platforms[1], CL_DEVICE_TYPE_ALL, 1,
	//	&device_id, &ret_num_devices);
	//printf("ret at %d is %d\n", __LINE__, ret);

	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
}

//Draw the circles/birds
void Flock::drawBird(Bird bird, float* allCircleVertices)
{
	//std::cout << "Position: " << bird.position[0] << ", " << bird.position[1] << ", " << bird.z << ", " << std::endl;
	int numberOfVertices = bird.numberOfSides + 2;

	float twicePi = 2.0f * M_PI;

	float* circleVerticesX = new float[numberOfVertices];
	float* circleVerticesY = new float[numberOfVertices];
	float* circleVerticesZ = new float[numberOfVertices];

	circleVerticesX[0] = bird.position[0];
	circleVerticesY[0] = bird.position[1];
	circleVerticesZ[0] = bird.z;

	for (int i = 1; i < numberOfVertices; i++)
	{
		circleVerticesX[i] = bird.position[0] + (bird.radius * cos(i *  twicePi / bird.numberOfSides));
		circleVerticesY[i] = bird.position[1] + (bird.radius * sin(i * twicePi / bird.numberOfSides));
		circleVerticesZ[i] = bird.z;
	}
	for (int i = 0; i < numberOfVertices; i++)
	{
		allCircleVertices[i * 3] = circleVerticesX[i];
		allCircleVertices[(i * 3) + 1] = circleVerticesY[i];
		allCircleVertices[(i * 3) + 2] = circleVerticesZ[i];
	}

	delete[] circleVerticesX;
	delete[] circleVerticesY;
	delete[] circleVerticesZ;
}

//Determine the birds next position
void Flock::calculateBird(int i, Bird* tbbBirds) {
	float* v1, *v2, *v3, *v4;

	v1 = rule1(tbbBirds[i], tbbBirds);
	v2 = rule2(tbbBirds[i], tbbBirds);
	v3 = rule3(tbbBirds[i], tbbBirds);
	v4 = rule4(tbbBirds[i]);

	tbbBirds[i].velocity[0] = tbbBirds[i].velocity[0] + v1[0] + (v2[0] / 5) + v3[0] + v4[0];
	tbbBirds[i].velocity[1] = tbbBirds[i].velocity[1] + v1[1] + (v2[1] / 5) + v3[1] + v4[1];

	//This is restrict the maximum velocity of the tbbBirds
	if (tbbBirds[i].velocity[0] > 10)
		tbbBirds[i].velocity[0] = 10;
	else if (tbbBirds[i].velocity[0] < -10)
		tbbBirds[i].velocity[0] = -10;

	if (tbbBirds[i].velocity[1] > 10)
		tbbBirds[i].velocity[1] = 10;
	else if (tbbBirds[i].velocity[1] < -10)
		tbbBirds[i].velocity[1] = -10;

	tbbBirds[i].position[0] = tbbBirds[i].position[0] + (tbbBirds[i].velocity[0]);
	tbbBirds[i].position[1] = tbbBirds[i].position[1] + (tbbBirds[i].velocity[1]);

	delete [] v1;
	delete [] v2;
	delete [] v3;
	delete [] v4;
}

//Steer towards center of other birds
float* Flock::rule1(Bird bird, Bird* tbbBirds) {
	float* avg = new float[2];
	avg[0] = 0;
	avg[1] = 0;

	for (int i = 0; i < NUM_BIRDS; i++) {
		if (bird.uuid != tbbBirds[i].uuid) {
			avg[0] += tbbBirds[i].position[0];
			avg[1] += tbbBirds[i].position[1];
		}
	}

	avg[0] /= NUM_BIRDS - 1;
	avg[1] /= NUM_BIRDS - 1;

	avg[0] = (avg[0] - bird.position[0]) / 120;
	avg[1] = (avg[1] - bird.position[1]) / 120;

	return avg;
}

//Keep small distance from other birds
float* Flock::rule2(Bird bird, Bird* tbbBirds) {
	float* c = new float[2];
	c[0] = 0;
	c[1] = 0;

	for (int i = 0; i < NUM_BIRDS; i++) {
		if (bird.uuid != tbbBirds[i].uuid) {
			if (abs(tbbBirds[i].position[0] - bird.position[0]) < 20 &&
				abs(tbbBirds[i].position[1] - bird.position[1]) < 20) {
				c[0] = c[0] - (tbbBirds[i].position[0] - bird.position[0]);
				c[1] = c[1] - (tbbBirds[i].position[1] - bird.position[1]);
			}
		}
	}
	return c;
}

//Match direction with other birds
float* Flock::rule3(Bird bird, Bird* tbbBirds) {
	float* avg = new float[2];
	avg[0] = 0;
	avg[1] = 0;

	for (int i = 0; i < NUM_BIRDS; i++) {
		if (bird.uuid != tbbBirds[i].uuid) {
			avg[0] += tbbBirds[i].velocity[0];
			avg[1] += tbbBirds[i].velocity[1];
		}
	}

	avg[0] /= NUM_BIRDS - 1;
	avg[1] /= NUM_BIRDS - 1;

	avg[0] = (avg[0] - bird.velocity[0]) / 10;
	avg[1] = (avg[1] - bird.velocity[1]) / 10;

	return avg;
}

//Steer away from boundries
float* Flock::rule4(Bird bird) {
	float* pos = new float[2];
	pos[0] = 0;
	pos[1] = 0;

	if (bird.position[0] < 50)
		pos[0] = 15;
	else if (bird.position[0] > 980)
		pos[0] = -15;

	if (bird.position[1] < 50)
		pos[1] = 15;
	else if (bird.position[1] > 670)
		pos[1] = -15;

	return pos;
}