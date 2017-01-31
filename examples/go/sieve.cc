#include <memory>
#include <thread>

#include "../../chan.hh"

void generate(std::weak_ptr<chan::unbuffered_chan<int>> ch) {
	for (int i = 2 ; ; i++) {
		ch.lock()->send(i);
	}
}

void filter(
	std::weak_ptr<chan::unbuffered_chan<int>> in,
	std::weak_ptr<chan::unbuffered_chan<int>> out,
	int prime
) {
	for (int i = 0 ; ; i++) {
		int x = 0;
		in.lock()->recv(x);

		if (x % prime != 0) {
			out.lock()->send(x);
		}
	}
}

int main() {
	auto ch = std::make_shared<chan::unbuffered_chan<int>>();
	std::thread(generate, ch).detach();

	for (int i = 0 ; i < 10 ; i++) {
		int prime = 0;
		ch->recv(prime);
		printf("%d\n", prime);

		auto ch1 = std::make_shared<chan::unbuffered_chan<int>>();
		std::thread(filter, ch, ch1, prime).detach();

		ch = ch1;
	}
}
