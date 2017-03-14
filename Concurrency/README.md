# Concurrency
##Introduction

Concurrency can be defined as the ability to perform independent computation tasks in parallel with the purpose of improving performance and responsiveness. There are mainly two ways to achieve this:

1. Running multiple processes

The operative system provides to each proccess each own memory space. This implies that a process can´t modify the data belonging to another process. As a result, in case that two processes needs to communicate, they have to do it through standard channels(sockets, files, etc.). 

It is very common to use this model in distributed systems (grid/cluster computing, render farm, etc.)

2. Running multiple threads within the same processs (Multithreading)

The execution of each process can be splitted into many threads. Each thread runs independently from the others, but within the same memory space. As a result, in case that two threads needs to communicate, they have to do it through shared memory. Access to shared memory by different threads can be concurrent and as a result, race conditions may arise. Because of this, proper synchronization mechanism(locks, atomic variables, etc.) must be used to guarantee data consistency across different threads. 
It is very common to use this model in applications where they need to run many different tasks and being responsive at the same time.

In this project, we want to show an understanding of concurrency and in particular of multithreading by using the threading library of C++11. 

## Implementation

We have simulated a random point cloud generation. 

The point cloud is represented by just an sdt::vector that holds 100000000 points that are set randomly.
In order to get a performance improvement, the key part consist of splitting the random point generation process in different chunks.
Each chunk will basically execute the following function:

```c++
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
```

We have implement 3 types of optimisations:

### Worker threads

Each chunk will be assigned to a worker thread. 
We use std::thread to create a worker thread.
We join all of the worker threads to the main thread to make sure that the main thread waits for the result.

### Async tasks

Similar to worker threads, but instead we use std::async with the launch policy specified to std::launch::async.
The launch policy is actually the one that will make the main thread to wait for all the async tasks to be completed.

### ThreadPool

We have implemented a basic thread pool that handles a queue of tasks. Each task is submitted to the pool and it is pick up by an available worker thread. Whenever a task is submitted, a result is returned that wraps and std::future to the result.

The thread pool is initialised with a number of worker threads equal to the number of cores that are in the system minus one.
Those worker threads are sleeping and they are waked up when there is a task to be done. This is done by using a condition variable.
In order to avoid a race condition over the queue, whenever we want to access or modify it, a std::unique_lock is used to lock the mutex attached to the queue. 

##Results

The table below shows the resuls after 50 iterations for each optimisation type. The result was taken on a i7 processor with additional 109 processes running in the background.
 
| Optimisation         | Total time    | Average Time |
|----------------------|--------------:|-------------:|
|                      |               |              | 
| None(Sequential)     | 16m 56s 498ms | 20s 329ms    |
|                      |               |              | 
| 7 worker threads     | 6m 4s 934ms   | 7s 298ms     |
| 40 worker threads    | 5m 49s 427ms  | 6s 988ms     |
| 100 worker threads   | 6m 4s 558ms   | 7s 291ms     |
| 500 worker threads   | 6m 10s 543ms  | 7s 410ms     |
|                      |               |              |  
| 7 async tasks        | 6m 4s 800ms   | 7s 296ms     |
| 40 async tasks       | 6m 8s 553ms   | 7s 371ms     |
| 100 async tasks      | 5m 54s 89ms   | 7s 81ms      |
| 500 async tasks      | 5m 49s 60ms   | 6s 981ms     |
|                      |               |              | 
| ThreadPool 7 tasks   | 6m 7s 596ms   | 7s 351ms     |
| ThreadPool 40 tasks  | 6m 12s 94ms   | 7s 441ms     |
| ThreadPool 100 tasks | 6m 1s 339mss  | 7s 226ms     |
| ThreadPool 500 tasks | 5m 57s 525ms  | 7s 150ms     |


It is clear that we are getting a considerable improvement with any of the optimisation types against the sequential or not optimised version, for the same amount of data. This is mainly because we are using the different cores available to create the points.

However, there is not a significant difference between using an std::async over an std::thread. Even though an std::async is lighter than a thread, probably the launch policy is the one that forces to behave std::async similar to a thread.

Finally, we can oberve that the threadpool behaves better than creating worker threads the more tasks that are to be done. This is probably because all the time involve in the thread creation is gone. However, this is not the case against async tasks.
