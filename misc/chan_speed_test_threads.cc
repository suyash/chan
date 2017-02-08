/**
 * https://github.com/je-so/testcode/blob/master/chan_speed_test.c
 * https://gist.github.com/tylertreat/111b752eb1e3e5c2bb3f
 *
 * This uses std::thread
 * */

#include <thread>

#include "../chan.hh"

const int MAX_THREADS = 16;

#ifdef __APPLE__
const int RUN_SIZE = 50000;
#else
const int RUN_SIZE = 12500;
#endif

std::thread serverThreads[MAX_THREADS];
std::thread clientThreads[MAX_THREADS];

int _ = 0;

std::chrono::time_point<std::chrono::system_clock> startTimes[MAX_THREADS];
std::chrono::time_point<std::chrono::system_clock> endTimes[MAX_THREADS];

void server(chan::chan<int>& c, int id) {
	startTimes[id] = std::chrono::system_clock::now();
	for (int i = 0; i < RUN_SIZE; i++) {
		c << i;
	}
}

void client(chan::chan<int>& c, int id) {
	for (int i = 0; i < RUN_SIZE; i++) {
		c >> _;
	}
	endTimes[id] = std::chrono::system_clock::now();
}

void measure(int numThreads) {
	chan::unbuffered_chan<int> c;

	for (int i = 0; i < numThreads; i++) {
		serverThreads[i] = std::thread(server, std::ref(c), i);
		clientThreads[i] = std::thread(client, std::ref(c), i);
	}

	for (int i = 0; i < numThreads; i++) {
		serverThreads[i].join();
		clientThreads[i].join();
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
	    "chan: %d*%d send/recv time in ms: %lld (%f nr_of_msg/msec)\n",
	    numThreads, RUN_SIZE, ms, double(numThreads * RUN_SIZE) / ms);
#else
	printf(
	    "chan: %d*%d send/recv time in ms: %ld (%f nr_of_msg/msec)\n",
	    numThreads, RUN_SIZE, ms, double(numThreads * RUN_SIZE) / ms);
#endif
}

int main() {
	for (int numThreads = 1; numThreads <= MAX_THREADS; numThreads <<= 1) {
		measure(numThreads);
	}
}
