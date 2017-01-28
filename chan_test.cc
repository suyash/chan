#include <gtest/gtest.h>

#include "chan.hh"

#include <thread>

TEST(unbuffered_chan, integers) {
	chan::unbuffered_chan<int> c;

	std::thread t([&](){
		int x = 0;

		c >> x;
		ASSERT_EQ(1, x);

		c >> x;
		ASSERT_EQ(2, x);

		c >> x;
		ASSERT_EQ(3, x);
	});

	c << 1 << 2 << 3;

	t.join();
}

TEST(unbuffered_chan, strings) {
	chan::unbuffered_chan<std::string> c;

	std::thread t([&](){
		std::string x;

		c >> x;
		ASSERT_STREQ("unbuffered", x.c_str());

		c >> x;
		ASSERT_STREQ("chan", x.c_str());

		c >> x;
		ASSERT_STREQ("test", x.c_str());
	});

	c << "unbuffered" << "chan" << "test";

	t.join();
}

TEST(buffered_chan, integers) {
	chan::buffered_chan<int> c(3);

	std::thread t([&](){
		int x = 0;

		c >> x;
		ASSERT_EQ(1, x);

		c >> x;
		ASSERT_EQ(2, x);

		c >> x;
		ASSERT_EQ(3, x);
	});

	int x = 1, y = 2, z = 3;
	c << x << y << z;

	t.join();
}

TEST(buffered_chan, strings) {
	chan::buffered_chan<std::string> c(3);

	std::thread t([&](){
		std::string x;

		c >> x;
		ASSERT_STREQ("buffered", x.c_str());

		c >> x;
		ASSERT_STREQ("chan", x.c_str());

		c >> x;
		ASSERT_STREQ("test", x.c_str());
	});

	std::string s1 = "buffered", s2 = "chan", s3 = "test";
	c << s1 << s2 << s3;

	t.join();
}

// https://golang.org/ref/mem#tmp_7
TEST(unbuffered_chan, communication_test) {
	std::string a = "not hello, world";
	chan::unbuffered_chan<int> c;

	auto f = [&]() {
		a = "hello, world";

		int _ = 0;
		c >> _;
	};

	std::thread t(f);
	c << 0;

	ASSERT_STREQ("hello, world", a.c_str());

	t.join();
}

// https://golang.org/ref/mem#tmp_7
TEST(buffered_chan, communication_test) {
	std::string a = "not hello, world";
	chan::buffered_chan<int> c(10);
	int _ = 0;

	auto f = [&]() {
		a = "hello, world";

		c << _;
	};

	std::thread t(f);

	// this will block until the thread sends something across c
	c >> _;

	ASSERT_STREQ("hello, world", a.c_str());

	t.join();
}
