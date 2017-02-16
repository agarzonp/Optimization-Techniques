#include <iostream>
#include <random>
#include <vector>
#include <chrono>

#include "math/Math.h"

void CreatePoints(std::vector<agarzon::Vec3>& points, int numPositions);
void CalculateDistances(std::vector<float>& distances, const agarzon::Vec3& startPoint, const std::vector<agarzon::Vec3>& endPoints);

int main()
{
	int NUM_POINTS = 100000000;
	std::vector<agarzon::Vec3> points;
	std::vector<float> distances;

	std::chrono::time_point<std::chrono::system_clock> pointsCreationStart, pointsCreationEnd;
	std::chrono::time_point<std::chrono::system_clock> calculateDistancesStart, calculateDistancesEnd;

	// points creation
	pointsCreationStart = std::chrono::system_clock::now();
	CreatePoints(points, NUM_POINTS);
	pointsCreationEnd = std::chrono::system_clock::now();

	// distance calculations
	calculateDistancesStart = std::chrono::system_clock::now();
	CalculateDistances(distances, points[0], points);
	calculateDistancesEnd = std::chrono::system_clock::now();

	// print time results
	std::cout << "Points creation (seconds): " << std::chrono::duration_cast<std::chrono::seconds>(pointsCreationEnd - pointsCreationStart).count() << std::endl;
	std::cout << "Distance calculation (seconds): " << std::chrono::duration_cast<std::chrono::seconds>(calculateDistancesEnd - calculateDistancesStart).count() << std::endl;
	std::cout << "Total time (seconds): " << std::chrono::duration_cast<std::chrono::seconds>(calculateDistancesEnd - pointsCreationStart).count() << std::endl;
	std::cout << std::endl;
	std::cout << "Points creation (milliseconds): " << std::chrono::duration_cast<std::chrono::milliseconds>(pointsCreationEnd - pointsCreationStart).count() << std::endl;
	std::cout << "Distance calculation (milliseconds): " << std::chrono::duration_cast<std::chrono::milliseconds>(calculateDistancesEnd - calculateDistancesStart).count() << std::endl;
	std::cout << "Total time (milliseconds): " << std::chrono::duration_cast<std::chrono::milliseconds>(calculateDistancesEnd - pointsCreationStart).count() << std::endl;

	std::cout << "Press any key to continue";
	std::getchar();
	return 0;
}

void CreatePoints(std::vector<agarzon::Vec3>& positions, int numPositions)
{
	// reserve beforehand the positions needed 
	positions.reserve(numPositions);

	// seed mt19937 random number generator
	std::random_device rd;
	std::mt19937 generator(rd());

	// create a uniform distribution
	std::uniform_real_distribution<float> distribution(-1000000.0f, std::nextafterf(1000000.0f, FLT_MAX));

	// push random points
	for (int i = 0; i < numPositions; i++)
	{
		positions.push_back(agarzon::Vec3(distribution(generator), distribution(generator), distribution(generator)));
	}
}

void CalculateDistances(std::vector<float>& distances, const agarzon::Vec3& startPoint, const std::vector<agarzon::Vec3>& positions)
{
	distances.reserve(positions.size());

	for (size_t i = 0; i < positions.size(); i++)
	{
		float distance = agarzon::Distance(startPoint, positions[i]);
		distances.push_back(distance);
	}
}


