cc=g++
cppflags=-g -std=c++11
include=-I./depend/json-c/include
lib=-lpthread

src=./src/charles_log.o ./src/test.o
test_bin=./src/test

all:$(test_bin)

$(test_bin):$(src)
	$(cc) $^ -o $@ $(lib)

%.o:%.cpp
	$(cc) $(cppflags) $(include) $^ -c -o $@

clean:
	-rm -f $(src) $(test_bin)
