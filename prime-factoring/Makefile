all: example_factoriser example_QS RSA_testing

.PHONY: all clean

CC = g++
#NOTE: compiling with -O3 seems to increase some factorisation times by up to 1.3x
CFLAGS = -O2

clean:
	rm ./example_factoriser ./example_QS ./RSA_testing ./*.out

example_factoriser: src/example_factoriser.cpp src/factoriser_QS.hpp src/factoriser_basic.hpp src/factoriser_math.hpp src/factoriser.hpp
	$(CC) $(CFLAGS) $< -o $@

example_QS: src/example_QS.cpp src/factoriser_QS.hpp src/factoriser_basic.hpp src/factoriser_math.hpp
	$(CC) $(CFLAGS) $< -o $@

RSA_testing: src/RSA_testing.cpp src/factoriser_QS.hpp src/factoriser_basic.hpp src/factoriser_math.hpp
	$(CC) $(CFLAGS) $< -o $@

