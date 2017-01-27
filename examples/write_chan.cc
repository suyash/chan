#include "../chan.hh"

#include <thread>

void write(chan::write_chan<int>& c) {
	printf("writing: 2\n");
	c << 2;
}

int main() {
	chan::unbuffered_chan<int> c;

	std::thread t(write, std::ref(c));

	int x = 0;
	c >> x;
	printf("read: %d\n", x);

	t.join();
}
