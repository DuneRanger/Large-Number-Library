all: example

.PHONY: all

CC = g++
CFLAGS = -O0

example: obj/main.o
	$(CC) $(CFLAGS) $^ -o $@

obj/main.o : src/main.cpp src/factoriser_QS.hpp src/factoriser_basic.hpp src/factoriser_math.hpp src/factoriser.hpp
	$(CC) $(CFLAGS) -c $< -o $@

