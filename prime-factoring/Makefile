all: example

.PHONY: all

CC = g++
CFLAGS = -O0

example: obj/main.o obj/factoriser_QS.o obj/factoriser_basic.o obj/factoriser_math.o
	$(CC) $(CFLAGS) $^ -o $@

obj/main.o : src/main.cpp src/factoriser_QS.h
	$(CC) $(CFLAGS) -c $< -o $@

obj/factoriser_QS.o: src/factoriser_QS.cpp src/factoriser_QS.h
	$(CC) $(CFLAGS) -c $< -o $@

obj/factoriser_basic.o: src/factoriser_basic.cpp src/factoriser_basic.h
	$(CC) $(CFLAGS) -c $< -o $@

obj/factoriser_math.o: src/factoriser_math.cpp src/factoriser_basic.h
	$(CC) $(CFLAGS) -c $< -o $@


