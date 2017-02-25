#include <iostream>
#include <random>
#include <vector>
#include <chrono>

#include "math/Math.h"

// seed mt19937 random number generator
std::random_device rd;
std::mt19937 generator(rd());

// create a uniform distribution
std::uniform_real_distribution<float> distribution(-1000000.0f, std::nextafterf(1000000.0f, FLT_MAX));

enum class Optimisation
{
	OPTIMISATION_NONE,

	OPTIMISATION_THREADS,
	OPTIMISATION_THREAD_POOL,

	NUM_OPTIMISATIONS
};

void CreatePoints(std::vector<agarzon::Vec3>& points, size_t numPoints, Optimisation optimisation = Optimisation::OPTIMISATION_NONE);
void _CreatePoints(std::vector<agarzon::Vec3>& points, size_t numPoints);

int main()
{
	// The set of points
	size_t NUM_POINTS = 100000000;
	std::vector<agarzon::Vec3> points;

	printf("Allocating memory for %d points...\n", NUM_POINTS);
	points.resize(NUM_POINTS);
	printf("Memory allocated!\n\n");

	while (true)
	{
		printf("\n");
		printf("Create Random Points\n");
		printf("Optimisation None = 0\n");
		printf("Optimisation N Worker Threads = 1\n");
		printf("Optimisation ThreadPool = 2\n");
		printf("\n");
		printf("Select Optimisation (q = for quiting): ");

		char c;
		std::cin >> c;
		std::cin.ignore();
		if (c == 'q')
		{
			// quit
			break;
		}

		printf("Creating Points...\n");

		int o = atoi(&c);
		Optimisation optimisation = (o  < (int)Optimisation::NUM_OPTIMISATIONS) ? Optimisation(o) : Optimisation::OPTIMISATION_NONE;

		std::chrono::time_point<std::chrono::system_clock> pointsCreationStart, pointsCreationEnd;

		pointsCreationStart = std::chrono::system_clock::now();
		CreatePoints(points, NUM_POINTS, optimisation);
		pointsCreationEnd = std::chrono::system_clock::now();

		// print time results
		printf("\n");
		printf("Points created! Time (seconds): %llu\n", std::chrono::duration_cast<std::chrono::seconds>(pointsCreationEnd - pointsCreationStart).count());
		printf("Points created! Time (milliseconds): %llu\n", std::chrono::duration_cast<std::chrono::milliseconds>(pointsCreationEnd - pointsCreationStart).count());
		printf("\n");
	}

	return 0;
}

void CreatePoints(std::vector<agarzon::Vec3>& points, size_t numPoints, Optimisation optimisation /*= OPTIMISATION_NONE */)
{
	switch (optimisation)
	{
	case Optimisation::OPTIMISATION_NONE:
	{
		_CreatePoints(points, numPoints);
		break;
	}
	case Optimisation::OPTIMISATION_THREADS:
	{
		break;
	}
	case Optimisation::OPTIMISATION_THREAD_POOL:
	{
		break;
	}
	default:
		break;
	}
}

void _CreatePoints(std::vector<agarzon::Vec3>& points, size_t numPoints)
{
	// push random points
	for (size_t i = 0; i < numPoints; i++)
	{
		agarzon::Vec3& p = points[i];

		p.x = distribution(generator);
		p.y = distribution(generator);
		p.z = distribution(generator);
	}
}
