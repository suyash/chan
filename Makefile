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

# All Google Test headers.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

# DO NOT TWEAK
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# General Tasks

all : test examples

clean_test :
	rm -f *_test.o *_test.out

clean_examples :
	rm -f $(EXAMPLES_OBJECTS) $(EXAMPLES_EXECS)

clean : clean_test clean_examples
	rm -f gtest.a gtest_main.a gtest-all.o gtest_main.o

test: $(TESTS)

examples: $(EXAMPLES)

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

# Tasks for $(EXAMPLES_DIR)/tylertreat_chan/unbuffered

$(EXAMPLES_DIR)/tylertreat_chan/unbuffered : $(EXAMPLES_DIR)/tylertreat_chan/unbuffered.out
	./$<

$(EXAMPLES_DIR)/tylertreat_chan/unbuffered.out : $(EXAMPLES_DIR)/tylertreat_chan/unbuffered.cc
	$(CXX) $(CXXFLAGS) $< -o $@

# Tasks for $(EXAMPLES_DIR)/tylertreat_chan/buffered

$(EXAMPLES_DIR)/tylertreat_chan/buffered : $(EXAMPLES_DIR)/tylertreat_chan/buffered.out
	./$<

$(EXAMPLES_DIR)/tylertreat_chan/buffered.out : $(EXAMPLES_DIR)/tylertreat_chan/buffered.cc
	$(CXX) $(CXXFLAGS) $< -o $@

# Tasks for $(EXAMPLES_DIR)/tylertreat_chan/close

$(EXAMPLES_DIR)/tylertreat_chan/close : $(EXAMPLES_DIR)/tylertreat_chan/close.out
	./$<

$(EXAMPLES_DIR)/tylertreat_chan/close.out : $(EXAMPLES_DIR)/tylertreat_chan/close.cc
	$(CXX) $(CXXFLAGS) $< -o $@

# Tasks for $(EXAMPLES_DIR)/read_chan

$(EXAMPLES_DIR)/read_chan : $(EXAMPLES_DIR)/read_chan.out
	./$<

$(EXAMPLES_DIR)/read_chan.out : $(EXAMPLES_DIR)/read_chan.cc
	$(CXX) $(CXXFLAGS) $< -o $@

# Tasks for $(EXAMPLES_DIR)/write_chan

$(EXAMPLES_DIR)/write_chan : $(EXAMPLES_DIR)/write_chan.out
	./$<

$(EXAMPLES_DIR)/write_chan.out : $(EXAMPLES_DIR)/write_chan.cc
	$(CXX) $(CXXFLAGS) $< -o $@
