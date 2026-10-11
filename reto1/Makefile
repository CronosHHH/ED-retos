CXX ?= g++
CPPFLAGS += -Iinclude
CXXFLAGS ?= -O2 -Wall -Wextra -Wpedantic
CXXFLAGS += -std=c++17

.PHONY: all test clean

all: bin/reto1

bin/reto1: build/JuegoVida.o build/main.o | bin
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

bin/pruebas: build/JuegoVida.o build/test_juego_vida.o | bin
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

build/%.o: src/%.cpp | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

build/test_juego_vida.o: tests/test_juego_vida.cpp | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

build bin:
	mkdir -p $@

test: bin/pruebas
	./bin/pruebas

clean:
	rm -rf build bin

-include build/*.d
