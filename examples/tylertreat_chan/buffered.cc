/**
 * Port of https://github.com/tylertreat/chan/blob/master/examples/buffered.c
 * */

#include "../../chan.hh"

int main() {
	chan::buffered_chan<std::string> c(2);

	c << "buffered" << "channel";

	std::string message;

	c >> message;
	printf("received '%s'\n", message.c_str());

	c >> message;
	printf("received '%s'\n", message.c_str());
}
