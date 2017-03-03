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
	struct ResultBase
	{
		ResultBase() {}
		virtual ~ResultBase() {}
		virtual void* WaitForResult() = 0;
	};

	template <typename T>
	struct Result : public ResultBase
	{
		Result(std::future<T>&& future)
			: future(std::move(future))
		{
		}

		void* WaitForResult() override
		{
			result = std::move(Get());
			return &result;
		}

		template<typename T>
		T Get()
		{
			return future.get();
		}

		std::future<T> future;
		T result;
	};

	template <>
	struct Result<void> : public ResultBase
	{
		Result(std::future<void>&& future)
			: future(std::move(future))
		{
		}

		void* WaitForResult() override
		{
			Get();
			return nullptr;
		}

		void Get()
		{
			future.get();
		}

		std::future<void> future;
	};

	std::unique_ptr<ResultBase> result;
	
	int taskId = -1;

public:

	ThreadTaskResult() {}

	template<typename T>
	ThreadTaskResult(std::future<T>& future, int taskId_)
		: result( new Result<T>(std::move(future)))
		, taskId(taskId_)
	{
	}


	void WaitForResult(void*& out)
	{
		//printf("Thread %ull waiting for task %d\n", std::hash<std::thread::id>()(std::this_thread::get_id()), taskId);

		void* r = result->WaitForResult();
		if (out)
		{
			out = r;
		}

		//printf("Thread %ull got result and no longer waiting for task %d\n", std::hash<std::thread::id>()(std::this_thread::get_id()), taskId);
	}
};


// Will figure out later what ThreadTask really is, probably some kind of callable object
class ThreadTask 
{
	int id_ = -1;

	bool done = false;
	std::promise<bool> donePromise;

	// Wrap the function that will actually do the task 
	//
	struct ThreadTaskFunction
	{
		virtual void call() = 0; 
		virtual ~ThreadTaskFunction(){};
	};

	template <typename Function>
	struct ThreadTaskFunctionWrapper : public ThreadTaskFunction
	{
		Function f;

		ThreadTaskFunctionWrapper(Function&& f_) 
			: f(std::move(f_))
		{
		}

		~ThreadTaskFunctionWrapper()
		{
		}

		ThreadTaskFunctionWrapper& operator=(ThreadTaskFunctionWrapper&& other)
		{
			f = std::move(f);
			return *this;
		}

		void call() override 
		{ 
			f(); // Note: return type and arguments are wrapped if it was added with std::bind
		} 
	};

	std::unique_ptr<ThreadTaskFunction> func;

public:
	
	ThreadTask() = default;

	~ThreadTask() 
	{
	}

	template<typename Function>
	ThreadTask(Function&& f)
		: func(new ThreadTaskFunctionWrapper<Function>(std::move(f)))
	{
	}

	template<typename Function>
	ThreadTask(Function&& f, int id_) 
		: func(new ThreadTaskFunctionWrapper<Function>(std::move(f)))
		, id (id_)
	{
	}

	ThreadTask(ThreadTask&& other)
	{
		id_ = other.id_;
		done = other.done;
		donePromise = std::move(other.donePromise);
		func = std::move(other.func);
	}

	ThreadTask& operator=(ThreadTask&& other)
	{
		id_ = other.id_;
		done = other.done;
		donePromise = std::move(other.donePromise);
		func = std::move(other.func);

		return (*this);
	}

	void DoTask()
	{
		assert(!done);

		//printf("Processing Task %d by thread %ull\n", id_, std::hash<std::thread::id>()(std::this_thread::get_id()));

		// call the function
		func->call();

		//printf("Task %d done by thread %ull\n", id_, std::hash<std::thread::id>()(std::this_thread::get_id()));

		done = true;
		donePromise.set_value(done);
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
			task.DoTask();
		}
	}
};

#endif // !THREAD_POOL_H
