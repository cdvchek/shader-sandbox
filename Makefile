# Compiler is c++ clang
CXX = clang++
CC = clang

# Local, untracked variables (per developer)
-include makefile.local.env

ifndef GLFW_ROOT
$(error Please set GLFW_ROOT in makefile.local.env in same directory as Makefile: "GLFW_ROOT := C:/msys64/clang64")
endif

# Compile with all warnings and c++17 in mind
CXXFLAGS = -Wall -std=c++17 -MMD -MP -Iinclude -I"$(GLFW_ROOT)/include"
LDFLAGS = -L"$(GLFW_ROOT)/lib"

CFLAGS = -Wall -std=c17 -MMD -MP -Iinclude -I"$(GLFW_ROOT)/include"

# External Link
EXLINKS = -lglfw3 -lopengl32 -lgdi32 -luser32 -lshell32 -lwinmm -lassimp

# Just grab every cpp file in src
SRC_CPP = $(wildcard ./src/*.cpp)
SRC_C = include/glad.c
# Object files are routed to obj directory
OBJ_CPP = $(patsubst ./src/%.cpp, ./obj/%.o, $(SRC_CPP))
OBJ_C = $(patsubst include/%.c, ./obj/%.o, $(SRC_C))
OBJ = $(OBJ_CPP) $(OBJ_C)
# Adding .d files to track hpp files
DEPS = $(OBJ:.o=.d)

TARGET = ./build/main.exe

.PHONY: all clean

# Main rule which depends on target
all: $(TARGET)

# Built-in Make Variables:
# $@ -> Rule target
# $^ -> All prereqs
# $< -> First prereq

# Target rule depends on all the object files
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(EXLINKS)

# Object file rule to build .o files from .cpp
# Also ensures obj directory exists
./obj/%.o: ./src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

./obj/%.o: ./include/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-del /Q .\obj\*.o .\obj\*.d 2>nul
	-del /Q .\build\*.exe 2>nul

-include $(DEPS)