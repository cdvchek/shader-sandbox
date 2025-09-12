# Compiler is c++ clang
CXX = clang++

# Compile with all warnings and c++17 in mind
CXXFLAGS = -Wall -std=c++17 -MMD -MP -Iinclude

# External Link
EXLINKS = -lglfw3 -lopengl32 -lgdi32

# Just grab every cpp file in src
SRC = $(wildcard ./src/*.cpp)
# Object files are routed to obj directory
OBJ = $(patsubst ./src/%.cpp, ./obj/%.o, $(SRC))
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
	$(CXX) $(CXXFLAGS) -o $@ $^ $(EXLINKS)

# Object file rule to build .o files from .cpp
# Also ensures obj directory exists
./obj/%.o: ./src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-del /Q .\obj\*.o .\obj\*.d 2>nul
	-del /Q .\build\*.exe 2>nul

-include $(DEPS)