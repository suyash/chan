#include <cmath>
#include <thread>

#include "../../chan.hh"

void term(chan::chan<double>& ch, int k) {
	ch << (4.0 * std::pow(-1, k)) / (2 * k + 1);
}

double pi(int n) {
	chan::unbuffered_chan<double> ch;

	for (int k = 0 ; k <= n ; k++) {
		std::thread(term, std::ref(ch), k).detach();
	}

	double f = 0;
	for (int k = 0 ; k <= n ; k++) {
		double x = 0;
		ch >> x;
		f += x;
	}

	return f;
}

int main () {
	printf("%lf\n", pi(5000));
}
