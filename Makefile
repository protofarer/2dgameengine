CXX := g++
CXX_FLAGS := -Wall -Wfatal-errors
LANG_STD = -std=c++17
INCLUDE_PATH := -I"./libs"

SRC_DIR := src
SRCS := src/*.cpp libs/imgui/*.cpp $(wildcard $(SRC_DIR)/**/*.cpp) # SRCS := ./src/*.cpp ./src/Game/*.cpp ./src/Logger/*.cpp
LINKER_FLAGS := -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -llua5.3

EXECUTABLE := gameengine
OBJS := $(patsubst %.cpp, %.o, $(SRCS))

build:
# g++ \
# -Wall -std=c++17 -I./libs \
# src/*.cpp $(wildcard src/**/*.cpp) \
# -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -llua5.3 \
# -o gameengine;
	$(CXX) $(CXX_FLAGS) $(LANG_STD) $(INCLUDE_PATH) $(SRCS) $(LINKER_FLAGS) -o $(EXECUTABLE) 

run: $(EXECUTABLE)
	./$(EXECUTABLE)

go: build run

.PHONY: clean
clean:
	rm -f $(EXECUTABLE) $(OBJS)