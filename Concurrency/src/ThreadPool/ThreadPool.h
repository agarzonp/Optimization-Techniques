#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <thread>
#include <mutex>

#include <deque>
#include<vector>

#include "ThreadTask.h"
#include "ThreadTaskResult.h"

class ThreadPool
{
	// tasks
	std::deque<ThreadTask> tasks; // TODO: What about a thread safe queue?

	// ThredPool terminate condition
	std::atomic_bool terminate = false;
	
	// worker threads
	std::vector<std::thread> workerThreads;

	// tasks queue mutex
	std::mutex tasksQueueMutex;

	// condition variable to wake up any sleepy worker thread
	std::condition_variable newTaskCondition;

public:

	ThreadPool()
	{
		Init();
	};

	ThreadPool(const ThreadPool& other) = delete;
	ThreadPool& operator=(const ThreadPool& other) = delete;

	~ThreadPool() 
	{
		terminate = true;
		 
		// Wake up all threads so we are able to join all of them
		// All threads will stop looping in WorkerThreadLoop

		newTaskCondition.notify_all();
		for (auto& t : workerThreads)
		{
			if (t.joinable())
			{
				t.join();
			}
		}

		// Important: Do we need a mechanisim to cancel a task, complete all queued tast, etc.?
	};

	// Add a task to the queue
	template<typename Function>
	ThreadTaskResult AddTask(Function&& function)
	{
		// wrap the callable object into a packaged_task
		typedef typename std::result_of<Function()>::type ResultType;
		std::packaged_task< ResultType() > task(std::move(function));

		// loct the queue
		std::lock_guard<std::mutex> lock(tasksQueueMutex);

		// store the future result 
		ThreadTaskResult taskResult(task.get_future(), tasks.size());

		// add the task to the queue
		tasks.push_back(std::move(ThreadTask(std::move(task))));

		// notify to one thread waiting on new task condition
		newTaskCondition.notify_one();

		return taskResult;
	}

private:

	// Init the thread pool
	void Init()
	{
		// set the pool of worker threads
		unsigned numWorkerThreads = std::thread::hardware_concurrency() - 1;
		for (unsigned i = 0; i < numWorkerThreads; i++)
		{
			workerThreads.push_back(std::thread(&ThreadPool::WorkerThreadLoop, this));
		}
	}

	// Function that workers thread will be executing
	void WorkerThreadLoop()
	{
		while (!terminate)
		{
			std::unique_lock<std::mutex> lock(tasksQueueMutex);

			// Wait until there is a new task
			// Execution from spurios wake up avoided by the lambda function. 
			// The thread will wait if there is no task at all or no need to terminate
			newTaskCondition.wait(lock, [this]() { return !tasks.empty() || terminate; });

			if (terminate)
			{
				//printf("Terminating thread %d\n", std::hash<std::thread::id>()(std::this_thread::get_id()));
				break;
			}

			// When a thread is notified to wake up the mutex will be lock, so thread safe to front() and pop()

			ThreadTask task = std::move(tasks.front());
			tasks.pop_front();

			// No need to protect the queue anymore
			lock.unlock();

			// do the task
			task.Do();
		}
	}
};

#endif // !THREAD_POOL_H
