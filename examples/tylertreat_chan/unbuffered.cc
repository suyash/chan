/**
 * Port of https://github.com/tylertreat/chan/blob/master/examples/unbuffered.c
 * */

#include <string>
#include <thread>

#include "../../chan.hh"

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
