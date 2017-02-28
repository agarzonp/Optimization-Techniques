#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <thread>
#include <deque>
#include<vector>
#include <mutex>
#include <cassert>

// Will figure out later what ThreadTaskResult really is
class ThreadTaskResult
{
	std::future<bool> doneFuture;
	int taskId_ = -1;

public:

	ThreadTaskResult(){}

	ThreadTaskResult(ThreadTaskResult& other)
	{
		doneFuture = std::move(other.doneFuture);
		taskId_ = other.taskId_;
	}

	ThreadTaskResult(ThreadTaskResult&& other)
	{
		doneFuture = std::move(other.doneFuture);
		taskId_ = other.taskId_;
	}

	void Set(std::future<bool>& future, int taskId_)
	{
		doneFuture = std::move(future);
		this->taskId_ = taskId_;
	}

	void WaitForResult()
	{
		//printf("Thread %ull waiting for task %d\n", std::hash<std::thread::id>()(std::this_thread::get_id()), taskId_);
		bool done = doneFuture.get();
		assert(done);
		//printf("Thread %ull got result and no longer waiting for task %d\n", std::hash<std::thread::id>()(std::this_thread::get_id()), taskId_);
	}
};

// Will figure out later what ThreadTask really is, probably some kind of callable object
class ThreadTask 
{
	int id_ = -1;

	bool done = false;
	std::promise<bool> donePromise;

public:
	
	ThreadTask(int id) : id_(id){}

	ThreadTask(ThreadTask&& other)
	{
		id_ = other.id_;
		done = other.done;
		donePromise = std::move(other.donePromise);
	}

	void DoTask()
	{
		assert(!done);

		for (int i = 0; i < 5; i++)
		{
			printf("Processing Task %d by thread %ull\n", id_, std::hash<std::thread::id>()(std::this_thread::get_id()));
		}
		printf("Task %d done by thread %ull\n", id_, std::hash<std::thread::id>()(std::this_thread::get_id()));

		done = true;
		donePromise.set_value(done);
	};

	
	void SetFutureResult(ThreadTaskResult& result)
	{
		result.Set(donePromise.get_future(), id_);
	}
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
	ThreadTaskResult AddTask(ThreadTask& task)
	{
		std::lock_guard<std::mutex> lock(tasksQueueMutex);
		tasks.push_back(std::move(task));

		// store future result
		ThreadTaskResult taskResult; 
		tasks.back().SetFutureResult(taskResult);

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

			task.DoTask(); // how do we synchronize if needed with the task submitter when the task is done?
		}
	}
};

#endif // !THREAD_POOL_H
