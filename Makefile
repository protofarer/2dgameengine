CXX := g++
LANG_STD = -std=c++17
CXX_FLAGS := -Wall -Wfatal-errors
INCLUDE_PATH := -I./libs
LD_FLAGS := -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -llua5.3

SRC_DIR := src
SRCS := src/*.cpp $(wildcard $(SRC_DIR)/**/*.cpp)
# SRCS := ./src/*.cpp ./src/Game/*.cpp ./src/Logger/*.cpp

EXECUTABLE := gameengine
OBJS := $(patsubst %.cpp, %.o, $(SRCS))

build:
# g++ \
# -Wall -std=c++17 -I./libs \
# src/*.cpp $(wildcard src/**/*.cpp) \
# -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -llua5.3 \
# -o gameengine;
	$(CXX) $(CXX_FLAGS) $(LANG_STD) $(INCLUDE_PATH) $(SRCS) $(LD_FLAGS) -o $(EXECUTABLE) 

run: $(EXECUTABLE)
	./$(EXECUTABLE)

go: build run

.PHONY: clean
clean:
	rm -f $(EXECUTABLE) $(OBJS)