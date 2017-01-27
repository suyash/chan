#include <gtest/gtest.h>

#include "circular_queue.hh"

TEST(circular_queue, basic) {
	chan::circular_queue<int> queue(2);

	queue.push(1);
	queue.push(2);

	ASSERT_EQ(1, queue.front());
	queue.pop();

	queue.push(3);

	ASSERT_EQ(2, queue.front());
	queue.pop();
	ASSERT_EQ(3, queue.front());
	queue.pop();
}
