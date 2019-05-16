#pragma once
#include <functional>
#include <vector>
#include <thread>
#include "Async.h"

namespace async {


	void parallel_for(int from, int to, int nrOfThreads, std::function<void(int)> func) {
		std::vector<std::thread> threads;
		threads.reserve(nrOfThreads);


		int blocks = (to - from) / nrOfThreads;

		for (int t = 0; t < nrOfThreads; t++)
		{
			threads.push_back(std::thread([&](int offset) -> void {
				int length = offset + blocks >= to ? to : offset + blocks;
				for (int i = offset; i < offset + blocks; i++)
				{
					func(i);
				}
			}, t * blocks));
		}

		for (auto& t : threads)
			t.join();
	}

	
};