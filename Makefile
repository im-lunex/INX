# INX - Infinity System Information
# Makefile for building the project

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = main
SRC = main.cpp

.PHONY: all clean run install

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o

run: $(TARGET)
	./$(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/inx
