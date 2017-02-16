#include <iostream>
#include <random>

#include "math/Math.h"



int main()
{
	// seed mt19937 random number generator
	std::random_device rd;
	std::mt19937 generator(rd());

	// create a uniform distribution
	std::uniform_real_distribution<float> distribution(-1000000.0f, std::nextafterf(1000000.0f, FLT_MAX));

	// generate random positions
	for (int i = 0; i < 10; i++)
	{
		agarzon::Vec3 posA(distribution(generator), distribution(generator), distribution(generator));
		agarzon::Vec3 posB(distribution(generator), distribution(generator), distribution(generator));
		printf("(%f, %f, %f)  , (%f, %f, %f) )\n", posA.x, posA.y, posA.z, posB.x, posB.y, posB.z);
	}
	
	std::cout << "Press any key to continue";
	std::getchar();
	return 0;
}
