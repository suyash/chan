#include <algorithm>
#include <random>
#include <thread>

#include "../../chan.hh"

struct tree {
	tree* left;
	int value;
	tree* right;

	tree(tree* left, int value, tree* right)
	: left(left), value(value), right(right) {}
};

void walk(tree* t, chan::chan<int>* ch) {
	if (!t) {
		return ;
	}

	walk(t->left, ch);
	ch->send(t->value);
	walk(t->right, ch);
}

chan::chan<int>* walker(tree* t) {
	auto ch = new chan::unbuffered_chan<int>();

	std::thread([ch, t](){
		walk(t, ch);
		ch->close();
	}).detach();

	return ch;
}

bool compare(chan::chan<int>* walker1, chan::chan<int>* walker2) {
	while (true) {
		int v1 = 0;
		auto ok1 = walker1->recv(v1);

		int v2 = 0;
		auto ok2 = walker2->recv(v2);

		if (!ok1 || !ok2) {
			return ok1 == ok2;
		}

		if (v1 != v2) {
			break;
		}
	}

	return false;
}

tree* insert(tree* t, int val) {
	if (!t) {
		return new tree(nullptr, val, nullptr);
	}

	if (t->value <= val) {
		return insert(t->left, val);
	}

	return insert(t->right, val);
}

template <typename URNG>
tree* newTree(int n, int k, URNG&& g) {
	int a[n];
	int x = 1;
	std::generate(a, a + n, [&](){ return x++; });
	std::shuffle(a, a + n, g);

	tree* t = nullptr;
	for (int i = 0 ; i < n ; i++) {
		t = insert(t, (a[i] + 1) * k);
	}

	return t;
}

void deleteTree(tree* t) {
	if (!t) {
		return ;
	}

	deleteTree(t->left);
	t->left = nullptr;

	deleteTree(t->right);
	t->right = nullptr;

	delete t;
}

int main() {
	auto t1 = newTree(100, 1, std::mt19937(4));
	auto w1 = walker(t1);
	auto t2 = newTree(100, 1, std::mt19937(4));
	auto w2 = walker(t2);

	if (compare(w1, w2)) {
		printf("same\n");
	} else {
		printf("different\n");
	}

	deleteTree(t2);
	delete w2;
	deleteTree(t1);
	delete w1;

	t1 = newTree(100, 1, std::random_device());
	w1 = walker(t1);
	t2 = newTree(100, 1, std::random_device());
	w2 = walker(t2);

	if (compare(w1, w2)) {
		printf("same\n");
	} else {
		printf("different\n");
	}

	deleteTree(t2);
	delete w2;
	deleteTree(t1);
	delete w1;
}
