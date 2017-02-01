#include <thread>

#include "../chan.hh"

bool ok = false;
bool sent = false;

void producer(chan::write_chan<int>* ch) {
	for (int i = 2 ; ok ; i++) {
		printf("w %d\n", i);
		sent = true;
		ch->send(i);
	}
}

void consumer(chan::read_chan<int>* ch) {
	for (int i = 0 ; ok || sent ; i++) {
		int x = 0;
		ch->recv(x);
		printf("r %d\n", x);
		sent = false;
	}
}

int main() {
	// auto ch = new chan::unbuffered_chan<int>();
	chan::unbuffered_chan<int> ch;

	ok = true;

	std::thread t1(producer, &ch);
	std::thread t2(consumer, &ch);

	std::this_thread::sleep_for(std::chrono::milliseconds(1));

	ok = false;

	t1.join();
	t2.join();
}
