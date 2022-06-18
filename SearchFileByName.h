#include <exception>
#include <filesystem>
#include <string>
#include <condition_variable>
#include <functional>
#include <future>
#include <vector>
#include <thread>
#include <queue>

class ThreadPool
{
public:
	using Task = std::function<void()>;

	explicit ThreadPool(std::size_t numThreads)
	{
		start(numThreads);
	}
	~ThreadPool()
	{
		stop();
	}

	template<class T>
	auto enqueue(T task)->std::future<decltype(task())>
	{
		auto wrapper = std::make_shared<std::packaged_task<decltype(task()) ()>>(std::move(task));

		{
			std::unique_lock<std::mutex> lock{ mEventMutex };
			mTasks.emplace([=] {
				(*wrapper)();
			});
		}

		mEventVar.notify_one();
		return wrapper->get_future();
	}

private:
	std::vector<std::thread> mThreads;
	std::condition_variable mEventVar;
	std::mutex mEventMutex;
	bool mStopping = false;
	std::queue<Task> mTasks;

	void start(std::size_t numThreads)
	{
		for (auto i = 0u; i < numThreads; ++i)
		{
			mThreads.emplace_back([=] {
				while (true)
				{
					Task task;

					{
						std::unique_lock<std::mutex> lock{ mEventMutex };

						mEventVar.wait(lock, [=] { return mStopping || !mTasks.empty(); });

						if (mStopping && mTasks.empty())
							break;

						task = std::move(mTasks.front());
						mTasks.pop();
					}

					task();
				}
			});
		}
	}

	void stop() noexcept
	{
		{
			std::unique_lock<std::mutex> lock{ mEventMutex };
			mStopping = true;
		}

		mEventVar.notify_all();

		for (auto& thread : mThreads)
			thread.join();
		mThreads.clear();
	}
};

void search_file(std::string directory_name, std::string file_name, std::string& file_path)
{
	for (const std::filesystem::directory_entry& p : std::filesystem::directory_iterator(directory_name))
	{
		try {
			if (!std::filesystem::is_regular_file(p.status())) {
				search_file(p.path().generic_string(), file_name, file_path);
				if (file_path != "")
					break;
			}

			if (file_name == p.path().filename()) {
				file_path = p.path().generic_string();
				break;
			}
		}
		catch (std::exception & e) {}
	}
}

std::string controller(std::string directory_name, std::string file_name) {
	try
	{
		ThreadPool pool{ 8 };
		std::string file_path = "";
		for (const std::filesystem::directory_entry& p : std::filesystem::directory_iterator(directory_name))
		{
			pool.enqueue([p, file_name, &file_path] {
				search_file(p.path().generic_string(), file_name, file_path);
			});
		}
		pool.~ThreadPool();
		if (file_path != "")
			return file_path;
	}
	catch (std::exception & e) {}
	return "File not found!";
}