#pragma once

#include <condition_variable>
#include <exception>
#include <mutex>

#include "circular_queue.hh"

namespace chan {

struct buffered_chan_rvalue_exception: public std::exception {
	virtual const char* what() const throw() {
		return "cannot add rvalues to buffered channels for now";
	}
} _buffered_chan_rvalue_exception;

template <typename T>
class chan;

/**
 * read_chan defines an interface for a channel that only supports reads
 *
 * example usage:
 *
 * ```
 * void read(chan::read_chan<int>& c) {
 * 	int x = 0;
 * 	c >> x;
 * 	printf("read: %d\n", x);
 * }
 *
 * int main() {
 * 	chan::unbuffered_chan<int> c;
 *
 * 	std::thread t(read, std::ref(c));
 *
 * 	printf("writing: 2\n");
 * 	c << 2;
 *
 * 	t.join();
 * }
 * ```
 * */
template <typename T>
class read_chan {
public:
	virtual bool close() = 0;
	virtual bool isClosed() const = 0;
	virtual bool read(T&) = 0;
	virtual chan<T>& operator>>(T&) = 0;
};

/**
 * write_chan defines an interface for a channel that only supports writes
 *
 * example usage:
 *
 * ```
 * void write(chan::write_chan<int>& c) {
 * 	printf("writing: 2\n");
 * 	c << 2;
 * }
 *
 * int main() {
 * 	chan::unbuffered_chan<int> c;
 *
 * 	std::thread t(write, std::ref(c));
 *
 * 	int x = 0;
 * 	c >> x;
 * 	printf("read: %d\n", x);
 *
 * 	t.join();
 * }
 * ```
 * */
template <typename T>
class write_chan {
public:
	virtual bool close() = 0;
	virtual bool isClosed() const = 0;
	virtual bool write(const T&) = 0;
	virtual chan<T>& operator<<(const T&) = 0;
	virtual chan<T>& operator<<(T&& val) = 0;
};

/**
 * chan defines the common interface for a channel supporting reads and writes.
 * It defines all required functions except for read and write, which are
 * implemented individually by buffered_chan and unbuffered_chan.
 *
 * The abscence of these 2 functions makes this an abstract type, so it cannot
 * be instantiated.
 * */
template <typename T>
class chan : public read_chan<T>, public write_chan<T> {
protected:
	bool is_closed;

	mutable std::mutex data_mutex;

	int read_wait_count;
	int write_wait_count;

	mutable std::condition_variable read_available;
	mutable std::condition_variable write_available;

public:
	chan() : is_closed(false), read_wait_count(0), write_wait_count(0) {}

	/**
	 * close will close a channel if it wasn't already closed, otherwise it
	 * will simply return false.
	 *
	 * @return  bool   the result of the close operation
	 * */
	bool close() {
		std::unique_lock<std::mutex> data_lock(data_mutex);

		if (is_closed) {
			return false;
		}

		is_closed = true;

		// notify any waiting read and write condition variables
		read_available.notify_all();
		write_available.notify_all();

		return true;
	}

	/**
	 * isClosed will return a boolean value indicating whether the channel
	 * has been closed or not.
	 *
	 * @return  bool   the current status
	 * */
	bool isClosed() const {
		std::unique_lock<std::mutex> data_lock(data_mutex);
		return is_closed;
	}

	/**
	 * operator<<(const T&) overloads the left shift operator to write
	 * to the channel, similar to std::ostream
	 * */
	virtual chan<T>& operator<<(const T& val) {
		this->write(val);
		return *this;
	}

	/**
	 * operator<<(T&&) implements writing rvalue references to a channel.
	 *
	 * unbuffered channels block the current thread after write until the
	 * data has been read by another thread. Hence, calling `write` directly
	 * with an rvalue is safe for them.
	 *
	 * For buffered channels, it isn't, hence this method has been overriden.
	 * */
	virtual chan<T>& operator<<(T&& val) {
		this->write(val);
		return *this;
	}

	/**
	 * operator>> overloads the right shift operator to read from the
	 * channel, similar to std::istream
	 * */
	chan<T>& operator>>(T& val) {
		this->read(val);
		return *this;
	}
};

/**
 * unbuffered_chan implements chan with unbuffered bidirectional reads and
 * writes.
 *
 * In addition to synchronizing data, it also synchronizes reads and
 * writes and at a time only one thread is allowed to read or write data.
 *
 * "Unbuffered channels combine communication—the exchange of a value—
 * with synchronization—guaranteeing that two calculations (goroutines)
 * are in a known state."
 *
 * - https://golang.org/doc/effective_go.html#channels
 * */
template <typename T>
class unbuffered_chan : public chan<T> {
private:
	const T* data;

	mutable std::mutex read_mutex;
	mutable std::mutex write_mutex;

public:
	unbuffered_chan() : data(nullptr) {}

	unbuffered_chan(const unbuffered_chan& other) = delete;
	unbuffered_chan& operator=(const unbuffered_chan& other) = delete;
	unbuffered_chan(unbuffered_chan&& other) = delete;
	unbuffered_chan& operator=(unbuffered_chan&& other) = delete;

	/**
	 * write here implements writing to an unbuffered channel in a manner
	 * that is semantically similar to the mechanism in golang.
	 *
	 * It first synchronizes write, then data.
	 *
	 * It proceeds with first writing the value, and then proceeding to
	 * notify any waiting readers, following which, if the data still hasn't
	 * been consumed, it blocks the current thread waiting to be notified on
	 * the data being consumed.
	 *
	 * "A send on a channel happens before the corresponding receive from
	 * that channel completes."
	 *
	 * - https://golang.org/ref/mem#tmp_7
	 *
	 *
	 * @param   const T&   the value to add
	 *
	 * @return  bool       true if the mission was a success
	 * */
	bool write(const T& val) {
		std::unique_lock<std::mutex> write_lock(write_mutex);
		std::unique_lock<std::mutex> data_lock(this->data_mutex);

		if (this->is_closed) {
			// TODO: figure out error here
			return false;
		}

		data = &val;
		this->write_wait_count++;

		if (this->read_wait_count > 0) {
			this->read_available.notify_one();
		}

		// wait until data is consumed
		while (!this->is_closed && data != nullptr) {
			this->write_available.wait(data_lock);
		}

		return true;
	}

	/**
	 * read first synchronizes the read, then data. Then it first waits for
	 * a write to happen, following which if the channel is still open, it
	 * will consume the data and notify any blocked writers to unblock
	 * themselves.
	 *
	 * "A receive from an unbuffered channel happens before the send on that
	 * channel completes."
	 *
	 * - https://golang.org/ref/mem#tmp_7
	 *
	 *
	 * @param   T&    the reference that is assigned the value in the front
	 *
	 * @return  bool  the result of this mission
	 * */
	bool read(T& valref) {
		std::unique_lock<std::mutex> read_lock(read_mutex);
		std::unique_lock<std::mutex> data_lock(this->data_mutex);

		while (!this->is_closed && this->write_wait_count == 0) {
			this->read_wait_count++;
			this->read_available.wait(data_lock);
			this->read_wait_count--;
		}

		if (this->is_closed) {
			// TODO: figure out error here
			valref = T(0);
			return false;
		}

		valref = *data;
		data = nullptr;

		this->write_wait_count--;
		this->write_available.notify_one();

		return true;
	}
};

/**
 * buffered_chan implements a buffered channel that supports multiple read/write
 * operations, upto a fixed "capacity".
 *
 * It can be used as a semaphore, to perform operations in batches.
 * */
template <typename T>
class buffered_chan : public chan<T> {
private:
	int capacity;
	circular_queue<const T*> data;

public:
	buffered_chan(int capacity)
	    : capacity(capacity), data(circular_queue<const T*>(capacity)) {
		// TODO: handle error when capacity is 0
	}

	buffered_chan(const buffered_chan& other) = delete;
	buffered_chan& operator=(const buffered_chan& other) = delete;
	buffered_chan(buffered_chan&& other) = delete;
	buffered_chan& operator=(buffered_chan&& other) = delete;

	/**
	 * write implements writing to a buffered channel. There is no call
	 * synchronization here, we only synchronize access to data. First we
	 * wait until the buffer is full. Then, if the channel isn't closed, we
	 * add the value to the channel's buffer and notify any waiting readers.
	 *
	 *
	 * @param   const T&   the value to add
	 *
	 * @return  bool       true if the mission was a success
	 * */
	bool write(const T& val) {
		std::unique_lock<std::mutex> data_lock(this->data_mutex);

		while (!this->is_closed && data.size() == capacity) {
			this->write_wait_count++;
			this->write_available.wait(data_lock);
			this->write_wait_count--;
		}

		if (this->is_closed) {
			// TODO: handle error here
			return false;
		}

		data.push(&val);

		// signal waiting reader
		if (this->read_wait_count > 0) {
			this->read_available.notify_one();
		}

		// NOTE: this doesn't immediately block for read

		return true;
	}

	/**
	 * operator<<(const T& val) calls write for lvalues.
	 *
	 * This is here because otherwise near match will always resolve to
	 * operator<<(T&&)
	 * */
	chan<T>& operator<<(const T& val) {
	       this->write(val);
	       return *this;
	}

	/**
	 * operator<< implements writing rvalue references to a buffered_chan.
	 *
	 * buffered channels do not block the current thread after write. Hence,
	 * the value might get destroyed before being read.
	 *
	 * For now, not supporting this by throwing an exception.
	 *
	 * TODO: figure this out.
	 * */
	chan<T>& operator<<(T&&) {
		throw _buffered_chan_rvalue_exception;
		return *this;
	}

	/**
	 * read implements data removal for a buffered channel. There is no call
	 * synchronization. It first blocks while the queue is empty. Once the
	 * queue is non-empty, it removes a value from the front of the queue
	 * and saves it in the passed reference, following which it notifies any
	 * waiting writers.
	 *
	 *
	 * @param   T&    the reference that is assigned the value in the front
	 *
	 * @return  bool  the result of this mission
	 * */
	bool read(T& valref) {
		std::unique_lock<std::mutex> data_lock(this->data_mutex);

		while (data.empty()) {
			if (this->is_closed) {
				// TODO: figure out error handling here
				valref = T(0);
				return false;
			}

			this->read_wait_count++;
			this->read_available.wait(data_lock);
			this->read_wait_count--;
		}

		valref = *(data.front());
		data.pop();

		if (this->write_wait_count > 0) {
			this->write_available.notify_one();
		}

		return true;
	}
};

}  // namespace chan
