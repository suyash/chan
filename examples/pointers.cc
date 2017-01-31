#include <memory>
#include <thread>

#include "../chan.hh"

void producer(std::shared_ptr<chan::unbuffered_chan<int>>& ch) {
	for (int i = 2 ; ; i++) {
		printf("w %d\n", i);
		ch->send(i);
	}
}

void consumer(std::shared_ptr<chan::unbuffered_chan<int>>& ch) {
	for (int i = 0 ; ; i++) {
		int x = 0;
		ch->recv(x);
		printf("r %d\n", x);
	}
}

int main() {
	auto ch = std::make_shared<chan::unbuffered_chan<int>>();

	std::thread t1(producer, std::ref(ch));
	std::thread t2(consumer, std::ref(ch));

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	t1.detach();
	t2.detach();
}
