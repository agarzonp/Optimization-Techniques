#ifndef THREAD_TASK_H
#define THREAD_TASK_H

#include<memory>

class ThreadTask
{
	// task id
	int id_ = -1;

	// Wrap the function that will actually do the task 
	//
	struct Task
	{
		virtual void call() = 0;
		virtual ~Task() {};
	};

	template <typename Function>
	struct TaskWrapper : public Task
	{
		Function f;

		TaskWrapper(Function&& f_)
			: f(std::move(f_))
		{
		}

		~TaskWrapper()
		{
		}

		TaskWrapper& operator=(TaskWrapper&& other)
		{
			f = std::move(f);
			return *this;
		}

		void call() override
		{
			f(); // Note: return type and arguments are wrapped
		}
	};

	std::unique_ptr<Task> func;

public:

	ThreadTask() = default;

	~ThreadTask()
	{
	}

	template<typename Function>
	ThreadTask(Function&& f)
		: func(new TaskWrapper<Function>(std::move(f)))
	{
	}

	template<typename Function>
	ThreadTask(Function&& f, int id_)
		: func(new TaskWrapper<Function>(std::move(f)))
		, id(id_)
	{
	}

	ThreadTask(ThreadTask&& other)
	{
		id_ = other.id_;
		func = std::move(other.func);
	}

	ThreadTask& operator=(ThreadTask&& other)
	{
		id_ = other.id_;
		func = std::move(other.func);

		return (*this);
	}

	void DoTask()
	{
		//printf("Processing Task %d by thread %ull\n", id_, std::hash<std::thread::id>()(std::this_thread::get_id()));

		// call the function
		func->call();

		//printf("Task %d done by thread %ull\n", id_, std::hash<std::thread::id>()(std::this_thread::get_id()));
	};
};

#endif // !THREAD_TASK_H

