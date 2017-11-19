#pragma once

#define _USE_MATH_DEFINES
#include <GL\glew.h>
#include <GL\glfw3.h>
#include <math.h>
#include <vector>
#include "flock.h"
#include <stdio.h>
#include <ctime>

//This can be increase to add more flocks
const int NUM_FLOCKS = 1;

class Window {
public:
	//Create a list of flocks
	std::vector<Flock> flocks = std::vector<Flock>();
	void start();

	struct tbbDrawFlocks;
};