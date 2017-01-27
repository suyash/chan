/**
 * Port of https://github.com/tylertreat/chan/blob/master/examples/unbuffered.c
 * */

#include "../../chan.hh"

#include <thread>

int main() {
	chan::unbuffered_chan<std::string> c;

	std::thread t([&](){
		c << "ping";
	});

	std::string message;
	c >> message;

	printf("received: %s\n", message.c_str());

	t.join();
}
