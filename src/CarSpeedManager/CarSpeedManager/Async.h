#pragma once
#include <functional>
#include <vector>
#include <thread>
#include <mutex>

namespace async {

	extern std::mutex lock;

	void parallel_for(int from, int to, int nrOfThreads, std::function<void(int)> func);

	template <typename T>
	void parallel_foreach(std::vector<T>& elements, int nrOfThreads, std::function<void(T&)> func) {
		std::vector<std::thread> threads;
		threads.reserve(nrOfThreads);


		int blocks = elements.size() / nrOfThreads;

		for (int t = 0; t < nrOfThreads; t++)
		{
			threads.push_back(std::thread([&](int offset) -> void {
				int length = offset + blocks >= elements.size() ? elements.size() : offset + blocks;
				for (int i = offset; i < offset + blocks; i++)
				{
					func(elements.at(i));
				}
			}, t * blocks));
		}

		for (auto& t : threads)
			t.join();
	}

};