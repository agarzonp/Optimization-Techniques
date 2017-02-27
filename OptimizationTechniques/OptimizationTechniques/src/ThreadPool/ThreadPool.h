#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <thread>
#include <deque>
#include<vector>


// Will figure out later what ThreadTask really is, probably some kind of callable object
class ThreadTask 
{
	int id_ = -1;

public:
	
	ThreadTask(int id) : id_(id){}

	void DoTask()
	{
		printf("Task %d done\n", id_);
	};
};

class ThreadPool
{
	// tasks
	std::deque<ThreadTask> tasks; // FIXME: Make it thread safe!

	// ThredPool terminate condition
	std::atomic_bool terminate = false;
	
	// worker threads
	std::vector<std::thread> workerThreads;

public:
	ThreadPool()
	{
		Init();
	};
	~ThreadPool() 
	{
		terminate = true;

		// make sure that any working thread executing a task finish that task 
		// Note: This doesn´t guarantee that all the tasks will be done!
		for (auto& t : workerThreads)
		{
			if (t.joinable())
			{
				t.join();
			}
		}

		// Important: Do we need mechanisim to cancel a task, complete all queued tast, etc.?
	};

	void AddTask(ThreadTask& task)
	{
		// FIXME: Make the lines below thread safe
		tasks.push_back(task);
	}

private:

	// Init the thread pool
	void Init()
	{
		// set the pool of worker threads
		unsigned numWorkerThreads = std::thread::hardware_concurrency() - 1;
		for (unsigned i = 0; i < numWorkerThreads; i++)
		{
			workerThreads.push_back(std::thread(&ThreadPool::WorkerThread, this));
		}
	}

	// Function that workers thread will be executing until there is a task
	void WorkerThread()
	{
		while (!terminate)
		{
			// FIXME: Make the lines below thread safe, so there are no race conditions!
			//
			if (tasks.size() > 0)
			{
				ThreadTask& task = tasks.front();
				tasks.pop_front();

				task.DoTask(); // how do we synchronize if needed with the submitter when the task is done?
			}
			else
			{
				// wait some time and allow other threads to run
				std::this_thread::yield();
			}
		}
	}
};

#endif // !THREAD_POOL_H
