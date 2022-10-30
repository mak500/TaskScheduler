/*****************************************************************//**
 * \file   scheduler.hh
 * \brief  Implements a scheduler.
 *
 * \author madhukar
 * \date   October 2022
 *********************************************************************/

#pragma once

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace thread {
	enum class ThreadState {
		Active,
		Terminate,
	};

	class Scheduler {
	private:
		unsigned int mThreadCount;
		unsigned int mThreadSchedId;

		std::vector<std::thread> mThreads;
		std::vector<ThreadState> mThreadState;

		std::vector<std::queue<std::function<void()>>> mJobs;
		std::mutex mJobsMutex;
		std::condition_variable mJobsCv;

	private:

		void runner(unsigned int threadId) {
			while (mThreadState[threadId] != ThreadState::Terminate) {
				std::function<void()> job;
				{
					std::unique_lock<std::mutex> lk(mJobsMutex);
					mJobsCv.wait(lk, [this, threadId]() { return this->mJobs[threadId].size() || mThreadState[threadId] == ThreadState::Terminate;  });
					if (mThreadState[threadId] == ThreadState::Terminate) {
						return;
					}
					job = mJobs[threadId].front();
					mJobs[threadId].pop();
				}
				job();
			}
		}

	public:
		Scheduler(unsigned int threadCount) : mThreadSchedId(0) {
			unsigned int hwThreadCount = std::thread::hardware_concurrency() - 1; // leave 1 thread for scheduler to run.

			mThreadCount = std::min(hwThreadCount, threadCount);
			mThreadState.reserve(mThreadCount);
			mJobs.reserve(mThreadCount);

			for (unsigned int threadId = 0; threadId < mThreadCount; threadId++) {
				mJobs.emplace_back(std::queue<std::function<void()>>());
				mThreadState.emplace_back(ThreadState::Active);
				mThreads.emplace_back(std::thread([this, threadId]() {
					this->runner(threadId);
					}));
			}
		}

		template<typename Function, typename... Args>
		std::future<std::invoke_result_t<std::decay_t<Function>, std::decay_t<Args>...>>
			schedule(Function&& fn, Args&&... args) {
			using RetType = std::invoke_result_t<std::decay_t<Function>, std::decay_t<Args>...>;
			auto p = std::make_shared<std::promise<RetType>>();
			auto f = p->get_future();

			{
				std::unique_lock<std::mutex> lk(mJobsMutex);
				mJobs[mThreadSchedId].push([p, fn = std::forward<Function>(fn), ... args = std::forward<Args>(args)]() mutable {
					p->set_value(fn(args...));
				});
				mJobsCv.notify_all();
			}

			mThreadSchedId++;
			mThreadSchedId %= mThreadCount;

			return f;
		}

		~Scheduler() {
			std::fill(mThreadState.begin(), mThreadState.end(), ThreadState::Terminate);
			mJobsCv.notify_all();
			std::for_each(mThreads.begin(), mThreads.end(), [](std::thread& th) {
				th.join();
				});
		}
	};
}; // namespace thread