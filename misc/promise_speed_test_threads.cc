#include <cmath>
#include <future>
#include <vector>

const int MAX_THREADS = 16;

#ifdef __APPLE__
const int RUN_SIZE = 50000;
#else
const int RUN_SIZE = 12500;
#endif

std::promise<int> promises[5][MAX_THREADS][RUN_SIZE]; // log2(16) + 1 = 5

std::future<void> serverThreads[MAX_THREADS];
std::future<void> clientThreads[MAX_THREADS];

std::chrono::time_point<std::chrono::system_clock> startTimes[MAX_THREADS];
std::chrono::time_point<std::chrono::system_clock> endTimes[MAX_THREADS];

int _ = 0;

void server(std::promise<int>* c, int id) {
	startTimes[id] = std::chrono::system_clock::now();
	for (int i = 0; i < RUN_SIZE; i++) {
		_ = c[i].get_future().get();
	}
}

void client(std::promise<int>* c, int id) {
	for (int i = 0; i < RUN_SIZE; i++) {
		c[i].set_value(i);
	}
	endTimes[id] = std::chrono::system_clock::now();
}

void measure(int numThreads) {
	int pain = static_cast<int>(std::log2(numThreads));
	for (int i = 0; i < numThreads; i++) {
		serverThreads[i] = std::async(std::launch::async, server, promises[pain][i], i);
		clientThreads[i] = std::async(std::launch::async, client, promises[pain][i], i);
	}

	for (int i = 0; i < numThreads; i++) {
		serverThreads[i].wait();
		clientThreads[i].wait();
	}

	auto smallestStart = startTimes[0];
	for (int i = 1; i < numThreads; i++) {
		if (startTimes[i] < smallestStart) {
			smallestStart = startTimes[i];
		}
	}

	auto largestEnd = endTimes[0];
	for (int i = 1; i < numThreads; i++) {
		if (endTimes[i] > largestEnd) {
			largestEnd = endTimes[i];
		}
	}

	uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(
	                  largestEnd - smallestStart)
	                  .count();

#ifdef __APPLE__
	printf(
	    "promise: %d*%d send/recv time in ms: %lld (%f nr_of_msg/msec)\n",
	    numThreads, RUN_SIZE, ms, double(numThreads * RUN_SIZE) / ms);
#else
	printf(
	    "promise: %d*%d send/recv time in ms: %ld (%f nr_of_msg/msec)\n",
	    numThreads, RUN_SIZE, ms, double(numThreads * RUN_SIZE) / ms);
#endif
}

int main() {
	for (int numThreads = 1; numThreads <= MAX_THREADS; numThreads <<= 1) {
		measure(numThreads);
	}
}
