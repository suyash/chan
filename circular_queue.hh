#pragma once

namespace chan {

/**
 * circular_queue implements a simple fixed size
 * circular buffer supporting FIFO order insertion and deletion.
 * */
template <typename T>
class circular_queue {
private:
	int capacity;
	int filled;

	int f;
	int b;

	T* data;

public:
	circular_queue(int capacity)
	    : capacity(capacity), filled(0), f(0), b(-1) {
		data = new T[capacity];
	}

	~circular_queue() { delete[] data; }

	circular_queue(const circular_queue<T>& other) {
		capacity = other.capacity;
		filled = other.filled;
		f = other.f;
		b = other.b;

		data = new T[capacity];
		for (int i = 0; i < capacity; i++) {
			data[i] = other.data[i];
		}
	}

	circular_queue& operator=(const circular_queue<T>& other) {
		capacity = other.capacity;
		filled = other.filled;
		f = other.f;
		b = other.b;

		data = new T[capacity];
		for (int i = 0; i < capacity; i++) {
			data[i] = other.data[i];
		}
	}

	circular_queue(circular_queue<T>&& other) {
		capacity = other.capacity;
		other.capacity = 0;

		filled = other.filled;
		other.filled = 0;

		f = other.f;
		other.f = 0;

		b = other.b;
		other.b = -1;

		data = other.data;
		other.data = nullptr;
	}

	circular_queue& operator=(circular_queue<T>&& other) {
		capacity = other.capacity;
		other.capacity = 0;

		filled = other.filled;
		other.filled = 0;

		f = other.f;
		other.f = 0;

		b = other.b;
		other.b = -1;

		data = other.data;
		other.data = nullptr;
	}

	/**
	 * empty checks if the queue is empty
	 *
	 * @return  bool   true if the queue is empty
	 * */
	inline bool empty() { return filled == 0; }

	/**
	 * size counts the number of elements
	 *
	 * @return  int   the number of elements in the queue
	 * */
	inline int size() { return filled; }

	/**
	 * full checks if the queue is completely occupied
	 *
	 * @return  bool   true if the queue is full
	 * */
	inline bool full() { return filled == capacity; }

	/**
	 * push pushes a value to the front of the queue
	 *
	 * @param   val   const T&   the parameter to push in the queue
	 *
	 * @return        bool       true if the operation was successful, false
	 *                           otherwise
	 * */
	bool push(const T& val) {
		if (filled == capacity) {
			// TODO: figure out error handling here
			return false;
		}

		b = (b + 1) % capacity;
		data[b] = val;

		filled++;
		return true;
	}

	/**
	 * front reads and returns the current item at the front of the queue
	 *
	 * @return  T   the item at the front of the queue
	 * */
	T front() {
		if (filled == 0) {
			// TODO: figure out error handling here
			return T(0);
		}

		return data[f];
	}

	/**
	 * pop will remove the item at the front from the queue
	 *
	 * @return  bool   the result of the operation, true if successful
	 * */
	bool pop() {
		if (filled == 0) {
			// TODO: figure out error handling here
			return false;
		}

		f = (f + 1) % capacity;
		filled--;
		return true;
	}
};

}  // namespace chan
