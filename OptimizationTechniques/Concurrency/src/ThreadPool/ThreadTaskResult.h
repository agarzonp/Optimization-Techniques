#ifndef THREAD_TASK_RESULT_H
#define THREAD_TASK_RESULT_H

#include<future>

class ThreadTaskResult
{
	// task id
	int taskId = -1;

	// Wrap the std::future from which we are going to get the result
	// 
	struct TaskResult
	{
		virtual ~TaskResult() {}
		virtual void* WaitForResult() = 0;
	};

	template <typename T>
	struct TaskResultWrapper : public TaskResult
	{
		TaskResultWrapper() = default;

		TaskResultWrapper(const std::future<T>& future) = delete;
		TaskResultWrapper(std::future<T>&& future)
			: future(std::move(future))
		{
		}

		TaskResultWrapper& operator=(const TaskResultWrapper& other) = delete;
		TaskResultWrapper& operator=(TaskResultWrapper&& other)
		{
			future = std::move(other.future);
			result = std::move(other.result);
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

	// Template specialization to handle void type
	template <>
	struct TaskResultWrapper<void> : public TaskResult
	{
		TaskResultWrapper() = default;

		TaskResultWrapper(const std::future<void>& future) = delete;
		TaskResultWrapper(std::future<void>&& future)
			: future(std::move(future))
		{
		}

		TaskResultWrapper& operator=(const TaskResultWrapper& other) = delete;
		TaskResultWrapper& operator=(TaskResultWrapper&& other)
		{
			future = std::move(other.future);
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

	std::unique_ptr<TaskResult> result;

public:

	ThreadTaskResult() = default;

	ThreadTaskResult(const ThreadTaskResult& other) = delete;
	ThreadTaskResult(ThreadTaskResult&& other)
	{
		taskId = other.taskId;
		result = std::move(other.result);
	}

	template<typename T>
	ThreadTaskResult(std::future<T>&& future, int taskId_)
		: result(new TaskResultWrapper<T>(std::move(future)))
		, taskId(taskId_)
	{
	}

	ThreadTaskResult& operator=(const ThreadTaskResult& other) = delete;
	ThreadTaskResult& operator=(ThreadTaskResult&& other)
	{
		taskId = other.taskId;
		result = std::move(other.result);
		return *this;
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

#endif // !THREAD_TASK_RESULT_H
