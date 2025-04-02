CXX = clang++
CXXFLAGS = -std=c++23 -Wall -Wextra -O3

MAKEFILE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

all: test

test: tests.cpp
	$(CXX) $(CXXFLAGS) -o tests tests.cpp engine.cpp
	@start=$$(date +%s%N); ./tests; end=$$(date +%s%N); echo "Duration: $$((end - start)) ns"

submit: engine.cpp
	$(CXX) $(CXXFLAGS) -fPIC -c engine.cpp -o engine.o
	$(CXX) $(CXXFLAGS) -shared -o engine.so engine.o
	lll-bench $(MAKEFILE_DIR)engine.so

clean:
	rm -f tests engine.o engine.so
