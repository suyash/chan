#include <memory>
#include <thread>

#include "../../chan.hh"

void generate(std::shared_ptr<chan::unbuffered_chan<int>> ch) {
	for (int i = 2 ; ; i++) {
		ch->send(i);
	}
}

void filter(
	std::shared_ptr<chan::unbuffered_chan<int>> in,
	std::shared_ptr<chan::unbuffered_chan<int>> out,
	int prime
) {
	for (int i = 0 ; ; i++) {
		int x = 0;
		in->recv(x);

		if (x % prime != 0) {
			out->send(x);
		}
	}
}

int main() {
	auto ch = std::make_shared<chan::unbuffered_chan<int>>();
	std::thread t(generate, ch);
	t.detach();

	for (int i = 0 ; i < 10 ; i++) {
		int prime = 0;
		ch->recv(prime);
		printf("%d\n", prime);

		auto ch1 = std::make_shared<chan::unbuffered_chan<int>>();
		std::thread t1(filter, ch, ch1, prime);
		t1.detach();

		ch = ch1;
	}
}
