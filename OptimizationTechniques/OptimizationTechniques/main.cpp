#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <sstream>
#include <algorithm>
#include<thread>
#include<future>

#include "math/Math.h"
#include "ThreadPool/ThreadPool.h"

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
	OPTIMISATION_ASYNC_TASKS,
	OPTIMISATION_THREAD_POOL,

	NUM_OPTIMISATIONS
};

const char* optimisationStrings[] =
{
	"NONE",
	"N WORKER THREADS",
	"N ASYNC TASKS",
	"THREAD_POOL"
};

// function declarations
void CreatePoints(std::vector<agarzon::Vec3>& points, size_t numPoints, Optimisation optimisation);
void CreatePoints(std::vector<agarzon::Vec3>& points, size_t numPoints);
void CreatePoints(std::vector<agarzon::Vec3>& points, size_t numPoints, size_t numWorkerThreads);
void CreatePointsByTasks(std::vector<agarzon::Vec3>& points, size_t numPoints, size_t numAsyncTasks);
void _CreatePoints(std::vector<agarzon::Vec3>& points, size_t startIndex, size_t endIndex);

std::string GetTimeStr(TimePoint start, TimePoint end);

// main
int main()
{
	// The set of points
	size_t NUM_POINTS = 100000000;
	std::vector<agarzon::Vec3> points;

	size_t m = sizeof(agarzon::Vec3) * NUM_POINTS;

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

void DoTask(int id)
{
	for (int i = 0; i < 5; i++)
	{
		printf("Doing task %d\n", id);
	}

	printf("Task %d done!\n", id);
}

void DoTaskWithFloat(int id, float f)
{
	for (int i = 0; i < 5; i++)
	{
		printf("Doing task %d using float %f\n ", id, f);
	}

	printf("Task %d done!\n", id);
}

int DoTaskWithInteger(int id, int d)
{
	int value = 0;
	for (int i = 0; i < 5; i++)
	{
		printf("Doing task %d using integer %d. Value: %d\n ", id, d, value);
		value++;
	}

	value *= d;
	printf("Task %d done! Value: %d\n", id, value);

	return value;
}

// CreatePoints
void CreatePoints(std::vector<agarzon::Vec3>& points, size_t numPoints, Optimisation optimisation)
{
	switch (optimisation)
	{
	case Optimisation::OPTIMISATION_NONE:
	{
		// create points
		printf("Creating Points...\n");
		pointsCreationStart = std::chrono::system_clock::now();
		CreatePoints(points, numPoints);
		pointsCreationEnd = std::chrono::system_clock::now();
		break;
	}
	case Optimisation::OPTIMISATION_THREADS:
	{
		// get the number of worker threads from the user
		printf("Number of worker threads: ");
		char buffer[256];
		std::cin.getline(buffer, 256);
		size_t numWorkerThreads = std::max(std::stoi(buffer), 1);

		// create points
		printf("Creating Points...\n");
		pointsCreationStart = std::chrono::system_clock::now();
		CreatePoints(points, numPoints, numWorkerThreads);
		pointsCreationEnd = std::chrono::system_clock::now();

		break;
	}
	case Optimisation::OPTIMISATION_ASYNC_TASKS:
	{
		// get the number of async tasks from the user
		printf("Number of async tasks: ");
		char buffer[256];
		std::cin.getline(buffer, 256);
		size_t numTasks = std::max(std::stoi(buffer), 1);

		// create points
		printf("Creating Points...\n");
		pointsCreationStart = std::chrono::system_clock::now();
		CreatePointsByTasks(points, numPoints, numTasks);
		pointsCreationEnd = std::chrono::system_clock::now();

		break;
	}
	case Optimisation::OPTIMISATION_THREAD_POOL:
	{
		printf("Creating Points...\n");

		pointsCreationStart = std::chrono::system_clock::now();

		// Add tasks to the thread pool
		ThreadPool threadPool;
		std::vector<ThreadTaskResult> results;

		std::vector< std::function<void()> > functions; // FIXME: Do we really need to store the functions?
		functions.resize(12);
		for (int i = 0; i < 10; i += 3)
		{
			functions[i] = std::move(std::bind(DoTask, i));
			functions[i + 1] = std::move(std::bind(DoTaskWithFloat, i  + 1, float(i + 1)));
			functions[i + 2] = std::move(std::bind(DoTaskWithInteger, i + 2, i + 2));
			ThreadTaskResult resultA = threadPool.AddTask(std::move(functions[i]));
			ThreadTaskResult resultB = threadPool.AddTask(std::move(functions[i + 1]));
			ThreadTaskResult resultC = threadPool.AddTask(std::move(functions[i + 2]));
			results.push_back(std::move(resultA));
			results.push_back(std::move(resultB));
			results.push_back(std::move(resultC));
		}

		// wait for all the results
		void* r = nullptr;
		for (auto& result : results)
		{				
			result.WaitForResult(r);
		}

		pointsCreationEnd = std::chrono::system_clock::now();

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

// CreatePoints
void CreatePoints(std::vector<agarzon::Vec3>& points, size_t numPoints)
{
	_CreatePoints(points, 0, numPoints);
}


void PrintHello(int a)
{
	std::cout << "Hello" << std::endl;
}

// CreatePoints using worker threads
void CreatePoints(std::vector<agarzon::Vec3>& points, size_t numPoints, size_t numWorkerThreads)
{
	size_t numPointsInChunk = numPoints / numWorkerThreads;

	size_t startIndex = 0;
	size_t endIndex = numPoints;

	std::vector<std::thread> threads;
	threads.resize(numWorkerThreads);

	// create the threads
	for (size_t i = 0; i < numWorkerThreads; i++)
	{
		startIndex = i*numPointsInChunk;
		endIndex = startIndex + numPointsInChunk;

		endIndex = (i + 1 == numWorkerThreads) ? numPoints : endIndex; // make sure that we cover all the points

		threads[i] = std::thread(_CreatePoints, std::ref(points), startIndex, endIndex);
	}

	// join all the threads
	std::for_each(threads.begin(), threads.end(), [](std::thread & thread)
	{
		thread.join();
	});
}

// CreatePoints using async tasks
void CreatePointsByTasks(std::vector<agarzon::Vec3>& points, size_t numPoints, size_t numAsyncTasks)
{
	size_t numPointsInChunk = numPoints / numAsyncTasks;

	size_t startIndex = 0;
	size_t endIndex = numPoints;

	// the future for each task
	std::vector<std::future<void>> futures;
	futures.resize(numAsyncTasks);

	// create the tasks
	for (size_t i = 0; i < numAsyncTasks; i++)
	{
		startIndex = i*numPointsInChunk;
		endIndex = startIndex + numPointsInChunk;

		endIndex = (i + 1 == numAsyncTasks) ? numPoints : endIndex; // make sure that we cover all the points

		auto ft = std::async(std::launch::async, &_CreatePoints, std::ref(points), startIndex, endIndex);
		futures[i] = std::move(ft);
	}

	// At this point, current thread will wait to all the tasks to be done
	// This is mainly because the destructor of each future will force the execution of the task...
	// ... but only if the launch policy is specified as std::launch::async instead of std::launch::deferred or default (std::launch::async | std::launch::deferred)
}

// CreatePoints
void _CreatePoints(std::vector<agarzon::Vec3>& points, size_t startIndex, size_t endIndex)
{
	//size_t threadId = std::hash<std::thread::id>()(std::this_thread::get_id());
	//printf("Thread %ull\n", threadId);

	for (size_t i = startIndex; i < endIndex; i++)
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
