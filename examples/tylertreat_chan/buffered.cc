/**
 * Port of https://github.com/tylertreat/chan/blob/master/examples/buffered.c
 * */

#include <string>

#include "../../chan.hh"

int main() {
	chan::buffered_chan<std::string> c(2);

	std::string s1 = "buffered", s2 = "channel";
	c << s1 << s2;

	std::string message;

	c >> message;
	printf("received '%s'\n", message.c_str());

	c >> message;
	printf("received '%s'\n", message.c_str());
}
