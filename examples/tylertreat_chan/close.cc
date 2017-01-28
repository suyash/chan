/**
 * Port of https://github.com/tylertreat/chan/blob/master/examples/close.c
 * */

#include "../../chan.hh"

#include <thread>

int main() {
	chan::buffered_chan<int> jobs(5);
	chan::unbuffered_chan<int> done;

	std::thread worker([&](){
		// Process jobs until channel is closed.
		int job;
		while (jobs.read(job)) {
			printf("received job: %d\n", job);
		}

		// Notify that all jobs were received.
		printf("received all jobs\n");
		done << 1;
	});

	// Send 3 jobs over the jobs channel then close it.
	for (int i = 1; i <= 3; i++) {
		// need to copy i as values are written by reference
		// and by the time the values are read, i might be 4.
		int j = i;

		jobs << j;
		printf("sent job: %d\n", i);
	}
	jobs.close();

	// Wait for all jobs to be received.
	int x;
	done >> x;

	worker.join();
}
