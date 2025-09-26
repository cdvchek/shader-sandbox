#clang++ -static -Wall -std=c++17 -MMD -MP -Iinclude -c src/window.cpp -o obj/window.o Compiler is c++ clang
CXX = clang++

# Compile with all warnings and c++17 in mind
CXXFLAGS = -static -Wall -std=c++17 -MMD -MP -Iinclude

# External Link
EXLINKS = -lglfw3 -lopengl32 -lgdi32

# Just grab every cpp and cpp file in src
SRC_CPP = $(wildcard ./src/*.cpp)
SRC_C = $(wildcard ./src/*.c)

# Object files are routed to obj directory
OBJ_CPP = $(patsubst ./src/%.cpp, ./obj/%.o, $(SRC_CPP))
OBJ_C = $(patsubst ./src/%.c, ./obj/%.o, $(SRC_C))

OBJ = $(OBJ_C) $(OBJ_CPP)

# Adding .d files to track hpp files
DEPS = $(OBJ:.o=.d) 0

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
	$(CXX) $(CXXFLAGS) -o $@ $^ $(EXLINKS)

# Object file rule to build .o files from .cpp
# Also ensures obj directory exists
./obj/%.o: ./src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

./obj/%.o: ./src/%.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-del /Q .\obj\*.o .\obj\*.d 2>nul
	-del /Q .\build\*.exe 2>nul

-include $(DEPS)