CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Wno-unused-parameter -Iinclude
LIBS     = -lGL -lGLEW -lglfw -lm

SRC      = src/main.cpp
TARGET   = heist

all: $(TARGET)

$(TARGET): $(SRC) include/*.h
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
