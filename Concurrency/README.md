# Concurrency
## Introduction

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
Each chunk basically belongs to a subset of the points. The creation of those points is done by the following function:

```c++
std::random_device rd;
std::mt19937 generator(rd());
std::uniform_real_distribution<float> distribution(-1000000.0f, std::nextafterf(1000000.0f, FLT_MAX));

void _CreatePoints(std::vector<agarzon::Vec3>& points, size_t startIndex, size_t endIndex)
{
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
The launch policy is actually the one that makes the main thread to wait for all the async tasks to be completed before exiting from the scope where the async task was called. This is mainly because the destructor of each std::future belonging to the async task will force the task to be executed in another thread if it has not previously been executed. Note that if the launch policy was std::launch::deferred, it would be executed in the calling thread and not in a new thread, therefore the concurrency is lost.

### ThreadPool

We have implemented a basic thread pool that handles a queue of tasks. Each task is submitted to the pool and it is picked up by an available worker thread. 

The thread pool is initialised with a number of worker threads equal to the number of cores that are in the system minus one. Therefore, it is designed to have one thread per core, so it could scale well in systems that have more or fewer cores. The worker threads are sleeping and they are waked up when there is a task in the queue. This is done by using a condition variable. Alternatively, we handle spurius awakes and make the thread to sleep if the queue is empty:

```c++

while (!terminate)
{	
	std::unique_lock<std::mutex> lock(tasksQueueMutex);

	// Wait until there is a new task
	// Execution from spurios wake up avoided by the lambda function. 
	// The thread will wait if there is no task at all or no need to terminate
	newTaskCondition.wait(lock, [this]() { return !tasks.empty() || terminate; });
	
	// thread awake! Execute task
}
```

In order to avoid a race condition over the queue, whenever we want to access or modify it, a std::unique_lock is used to lock the mutex attached to the queue. The reason why a std::unique_lock is used over std::lock_guard is mainly because with the first one we can unlock the mutex whenever we want whereas with the latter only when it goes out of the scope. For example, we do not want to keep the mutex locked while the task is executing because there is no need to protect the queue for race condition at that time, therefore we want other threads to get access to the queue as earlier as possible.

Whenever a task (ThreadTask) is submitted, a result(ThreadTaskResult)is returned. This ThreadTask and ThreadTaskResult are actually a wrapper to a std::packaged_task and an std::future respectively. Therefore, the thread that submits the task waits until it has been executed. These two data structures are using templates, so we can submit and get results of any type(template specialization for the void case).

Finally, neither the threadpool or related data structures are copyable, move semantics are used instead. This decision is not only because of tperformance implication, but as well to keep consistency with the not copyable objects that we are using, mainly std::future and std::unique_ptr. 


## Results

The table below shows the results after 50 iterations for each optimisation type. 

The results were taken on a Intel Core i7-3630QM @2.4Ghz  processor (8 cores) and using Windows 7 as the operative system. In the background, there were running an additional of 109 processes apart from the current experiment.
 
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


It is clear that for the same amount of data, that is 100000000 points, we are getting a considerable improvement with any of the optimisation types against the sequential or not optimised version. This is mainly because we are using different cores available to create the points.

However, there is not a significant difference between using an std::async over an std::thread. Even though an std::async is lighter than a thread, the launch policy is the one that forces to behave std::async similar to a thread, as we said previously, it will force the task to be run in a different thread.

Finally, we can oberve that the threadpool behaves better than creating worker threads when there amount of tasks is very high. This is probably because the time that involves thread creation/destruction and the impact of the thread scheduling by the OS is minimised with the threadpool. Mainly, because there are less threads to handle. However, this is not the case against async tasks.

All in all, the threadpool looks like to be the multithreading optimisation technique to use, not only because it performs similiar to the rest of "primitive" techniques, but as well because it would scale better in different systems with different hardware capabilities
