#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <thread>
#include <deque>
#include<vector>

#include <mutex>


// Will figure out later what ThreadTask really is, probably some kind of callable object
class ThreadTask 
{
	int id_ = -1;

public:
	
	ThreadTask(int id) : id_(id){}

	void DoTask()
	{
		for (int i = 0; i < 5; i++)
		{
			printf("Processing Task %d by thread %d\n", id_, std::hash<std::thread::id>()(std::this_thread::get_id()));
		}
		printf("Task %d done by thread %d\n", id_, std::hash<std::thread::id>()(std::this_thread::get_id()));
	};
};

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
	void AddTask(ThreadTask& task)
	{
		std::lock_guard<std::mutex> lock(tasksQueueMutex);
		tasks.push_back(task);

		// notify to one thread waiting on this condition
		newTaskCondition.notify_one();
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

			ThreadTask task = tasks.front(); // move semantics needed?
			tasks.pop_front();

			// No need to protect the queue anymore
			lock.unlock();

			task.DoTask(); // how do we synchronize if needed with the task submitter when the task is done?
		}
	}
};

#endif // !THREAD_POOL_H
