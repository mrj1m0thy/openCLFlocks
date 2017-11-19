#define _USE_MATH_DEFINES
#pragma once
#include <math.h>
#include <list>
#include <iostream>
#include <tbb\parallel_for.h>
#include <tbb\blocked_range.h>
#include <CL/cl.h>
#include <boost\uuid\uuid.hpp>
#include <boost\uuid\random_generator.hpp>
#include <boost\uuid\uuid_io.hpp>
#include <string.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 720
#define MAX_SOURCE_SIZE (0x100000)

class Flock {
	int coordinateX;
	int coordinateY;

public:
	//This can be increased to add more birds to the flocks
	const static int NUM_BIRDS = 8;
	struct Bird
	{
		char* uuid;
		float* velocity = new float[2];
		float* position = new float[2];
		float z = 0;
		float radius = 5;
		int numberOfSides = 10;
	};
	Flock();
	~Flock();
	void static drawBird(Bird, float*);
	void drawFlock();
	void static calculateBird(int, Bird*);
	static float* rule1(Bird, Bird*);
	static float* rule2(Bird, Bird*);
	static float* rule3(Bird, Bird*);
	static float* rule4(Bird);

	void loadKernel(char*);
	void getPlatInfo();

	float** vertices;
	Bird*  birds;
	int numberOfVertices = 32;

	struct tbbDrawBirds;
	struct calculateBirdPositions;
	


	FILE *fp;
	char *source_str;
	size_t source_size;
	cl_device_id device_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_int ret;
	//cl_platform_id *platforms = NULL;
	cl_platform_id platform_id = NULL;

	cl_context context;
	cl_command_queue command_queue;

};