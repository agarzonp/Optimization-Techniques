#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <sstream>

#include "math/Math.h"

// seed mt19937 random number generator
std::random_device rd;
std::mt19937 generator(rd());

// create a uniform distribution
std::uniform_real_distribution<float> distribution(-1000000.0f, std::nextafterf(1000000.0f, FLT_MAX));

// time points
typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;
TimePoint pointsCreationStart, pointsCreationEnd;

// Optimisation type
enum class Optimisation
{
	OPTIMISATION_NONE,

	OPTIMISATION_THREADS,
	OPTIMISATION_THREAD_POOL,

	NUM_OPTIMISATIONS
};

const char* optimisationStrings[] =
{
	"NONE",
	"N WORKER THREADS",
	"THREAD_POOL"
};

// function declarations
void CreatePoints(std::vector<agarzon::Vec3>& points, size_t numPoints, Optimisation optimisation = Optimisation::OPTIMISATION_NONE);
void _CreatePoints(std::vector<agarzon::Vec3>& points, size_t numPoints);
std::string GetTimeStr(TimePoint start, TimePoint end);

// main
int main()
{
	// The set of points
	size_t NUM_POINTS = 100000000;
	std::vector<agarzon::Vec3> points;

	// allocate memory for NUM_POINTS
	printf("Allocating memory for %d points...\n", NUM_POINTS);
	points.resize(NUM_POINTS);
	printf("Memory allocated!\n\n");

	while (true)
	{
		// print menu
		printf("\n");
		printf("Create Random Points with Optimisation: \n\n");
		for (int i = 0; i < (int)Optimisation::NUM_OPTIMISATIONS; i++)
		{
			printf("%d = %s\n", i, optimisationStrings[i]);
		}
		printf("\nSelect Optimisation OR 'q' for quiting): ");

		// get optimiation type
		char c;
		std::cin >> c;
		std::cin.ignore();
		if (c == 'q') // quit
			break;

		int o = atoi(&c);
		Optimisation optimisation = (o  < (int)Optimisation::NUM_OPTIMISATIONS) ? Optimisation(o) : Optimisation::OPTIMISATION_NONE;

		// Create points
		CreatePoints(points, NUM_POINTS, optimisation);
	}

	return 0;
}

// CreatePoints
void CreatePoints(std::vector<agarzon::Vec3>& points, size_t numPoints, Optimisation optimisation /*= OPTIMISATION_NONE */)
{
	switch (optimisation)
	{
	case Optimisation::OPTIMISATION_NONE:
	{
		printf("Creating Points...\n");
		pointsCreationStart = std::chrono::system_clock::now();
		_CreatePoints(points, numPoints);
		pointsCreationEnd = std::chrono::system_clock::now();
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

	// print time results
	printf("\n");
	printf("Points created! Time: %s\n", GetTimeStr(pointsCreationStart, pointsCreationEnd).c_str());
	printf("\n");
}

// _CreatePoints
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

// GetTimeStr
std::string GetTimeStr(TimePoint start, TimePoint end)
{
	size_t total = (size_t)std::chrono::duration_cast<std::chrono::milliseconds>(pointsCreationEnd - pointsCreationStart).count();

	size_t seconds = total / 1000;
	size_t milliseconds = total - (seconds * 1000);

	std::stringstream ss;
	ss << seconds << "s" << " " << milliseconds << "ms";

	return ss.str();
}
