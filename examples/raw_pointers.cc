#include <thread>

#include "../chan.hh"

void producer(chan::write_chan<int>* ch) {
	for (int i = 0 ; i < 100 ; i++) {
		printf("w %d\n", i);
		ch->send(i);
	}
}

void consumer(chan::read_chan<int>* ch) {
	for (int i = 0 ; i < 100 ; i++) {
		int x = 0;
		ch->recv(x);
		printf("r %d\n", x);
	}
}

int main() {
	// auto ch = new chan::unbuffered_chan<int>();
	chan::unbuffered_chan<int> ch;

	std::thread t1(producer, &ch);
	std::thread t2(consumer, &ch);

	t1.join();
	t2.join();
}
