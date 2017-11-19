//__kernel void vector_add(__global const int *A, __global const int *B, __global int *C) {

	// Get the index of the current element to be processed
	//int i = get_global_id(0);
	// Do the operation
	//C[i] = A[i] + B[i];
//}

typedef struct __Bird
{
	char* uuid;
	float* velocity = new float[2];
	float* position = new float[2];
	float z = 0;
	float radius = 5;
	int numberOfSides = 10;
}Bird;

__kernel void calculateBird(__global Bird* tbbBirds) {
	int i = get_global_id(0);
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

	delete[] v1;
	delete[] v2;
	delete[] v3;
	delete[] v4;
}

//Steer towards center of other birds
float* rule1(Bird bird, Bird* tbbBirds) {
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
float* rule2(Bird bird, Bird* tbbBirds) {
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
float* rule3(Bird bird, Bird* tbbBirds) {
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
float* rule4(Bird bird) {
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