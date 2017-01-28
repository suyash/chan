/**
 * https://github.com/je-so/testcode/blob/master/chan_speed_test.c
 * https://gist.github.com/tylertreat/111b752eb1e3e5c2bb3f
 *
 * chan_speed_test using std::async instead of std::thread
 * */

#include <future>

#include "../chan.hh"

const int MAX_THREADS = 16;

std::future<void> serverThreads[MAX_THREADS];
std::future<void> clientThreads[MAX_THREADS];

int _ = 0;

std::chrono::time_point<std::chrono::system_clock> startTimes[MAX_THREADS];
std::chrono::time_point<std::chrono::system_clock> endTimes[MAX_THREADS];

void server(chan::chan<int>& c, int id) {
	startTimes[id] = std::chrono::system_clock::now();
	for (int i = 0; i < 1000000; i++) {
		c >> _;
	}
}

void client(chan::chan<int>& c, int id) {
	for (int i = 0; i < 1000000; i++) {
		c << i;
	}
	endTimes[id] = std::chrono::system_clock::now();
}

void measure(int numThreads) {
	chan::unbuffered_chan<int> c;

	for (int i = 0; i < numThreads; i++) {
		serverThreads[i] = std::async(std::launch::async, server, std::ref(c), i);
		clientThreads[i] = std::async(std::launch::async, client, std::ref(c), i);
	}

	int _ = 0;

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
	    "chan: %d*1000000 send/recv time in ms: %lld (%f nr_of_msg/msec)\n",
	    numThreads, ms, double(numThreads * 1000000) / ms);
#else
	printf(
	    "chan: %d*1000000 send/recv time in ms: %ld (%f nr_of_msg/msec)\n",
	    numThreads, ms, double(numThreads * 1000000) / ms);
#endif
}

int main() {
	for (int numThreads = 1; numThreads <= MAX_THREADS; numThreads <<= 1) {
		measure(numThreads);
	}
}