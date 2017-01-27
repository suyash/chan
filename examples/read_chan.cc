#include "../chan.hh"

#include <thread>

void read(chan::read_chan<int>& c) {
	int x = 0;
	c >> x;
	printf("read: %d\n", x);
}

int main() {
	chan::unbuffered_chan<int> c;

	std::thread t(read, std::ref(c));

	printf("writing: %d\n", 2);
	c << 2;

	t.join();
}
