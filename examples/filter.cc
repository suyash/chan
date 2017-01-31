#include <memory>
#include <thread>

#include "../chan.hh"

void producer(chan::write_chan<int>& ch) {
	for (int i = 1 ; ; i++) {
		ch << i;
	}
}

void filter(
	chan::read_chan<int>& in,
	chan::write_chan<int>& out,
	int f
) {
	for (int i = 0 ; ; i++) {
		int x = 0;
		in >> x;

		if (x % f == 0) {
			out << x;
		}
	}
}

int main() {
	chan::unbuffered_chan<int> ch;
	std::thread t(producer, std::ref(ch));
	t.detach();

	chan::unbuffered_chan<int> ch2;
	std::thread t2(filter, std::ref(ch), std::ref(ch2), 3);
	t2.detach();

	for (int i = 0 ; i < 10 ; i++) {
		int x = 0;
		ch2 >> x;

		printf("%d\n", x);
	}
}
