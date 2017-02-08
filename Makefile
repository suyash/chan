# Based on the Makefile in googletest/make

# Points to the root of Google Test
GTEST_DIR = vendor/googletest

# Pointers to the root directories of examples
EXAMPLES_DIR = examples

# Flags passed to the preprocessor.
CPPFLAGS += -isystem $(GTEST_DIR)/include -std=c++11

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -pthread -std=c++11

# All tests produced by this Makefile.
TESTS = circular_queue_test chan_test

# All examples produced by this Makefile
EXAMPLES_SRC = $(wildcard $(EXAMPLES_DIR)/**/*.cc $(EXAMPLES_DIR)/*.cc)
EXAMPLES_OBJECTS = $(EXAMPLES_SRC:.cc=.o)
EXAMPLES_EXECS = $(EXAMPLES_SRC:.cc=.out)
EXAMPLES = $(EXAMPLES_SRC:.cc=)

# Speed Tests
SPEED_TESTS = misc/chan_speed_test_async misc/chan_speed_test_threads misc/promise_speed_test_threads

# All Google Test headers.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

# DO NOT TWEAK
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# General Tasks

all : test examples speed_tests

clean_test :
	rm -f *_test.o *_test.out

clean_examples :
	rm -f $(EXAMPLES_OBJECTS) $(EXAMPLES_EXECS)

clean : clean_test clean_examples
	rm -f gtest.a gtest_main.a gtest-all.o gtest_main.o

test: $(TESTS)

examples: $(EXAMPLES)

speed_tests: $(SPEED_TESTS)

# Tasks for gtest

gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

# Tasks for circular_queue_test

circular_queue_test.o : circular_queue_test.cc $(GTEST_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c circular_queue_test.cc

circular_queue_test.out : gtest_main.a circular_queue_test.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@

circular_queue_test : circular_queue_test.out
	./$<

# Tasks for chan_test

chan_test.o : chan_test.cc $(GTEST_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c chan_test.cc

chan_test.out : gtest_main.a chan_test.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@

chan_test : chan_test.out
	./$<

# Utilize the default task for running examples

% : %.cc
	$(CXX) $(CXXFLAGS) $< -o $@.out
	./$@.out
