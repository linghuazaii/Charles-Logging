cc=g++
cppflags=-std=c++11 -fPIC
include=-I./depend/json-c/include
lib=-L./depend/json-c/lib \
	 -ljson-c

src=./src/charles_log.o 
so=./lib/libcharles_log.so.1
test_bin=./bin/test

all:$(so) $(test_bin)

$(so):$(src)
	$(cc) -shared -fPIC -Wl,-soname,libcharles_log.so.1 $^ -o $@ $(lib)
	cd lib && ln -s libcharles_log.so.1 libcharles_log.so
	cp -rd depend/json-c/lib/libjson-c.so* lib/
	cp src/charles_log.h include/

%.o:%.cpp
	$(cc) $(cppflags) $(include) $^ -c -o $@

$(test_bin):
	$(cc) -L./lib src/test.cpp -o $@ -lpthread -ljson-c -lcharles_log -std=c++11

clean:
	-rm -rf $(src) $(test_bin) ./lib/* ./include/*
