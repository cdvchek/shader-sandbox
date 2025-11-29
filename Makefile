CXX = clang++
CC = clang

IMGUI_DIR = include/imgui
# Local, untracked variables (per developer)
-include makefile.local.env

# Detect OS
OS := $(shell uname -s)

ifeq ($(OS),Linux)
    # WSL/Linux path
    ifndef GLFW_ROOT_WSL
        $(error Please set GLFW_ROOT_WSL in makefile.local.env: "GLFW_ROOT_WSL := /mnt/e/1Home/.../clang64")
    endif
    GLFW_ROOT := $(GLFW_ROOT_WSL)
else
    # Windows path
    ifndef GLFW_ROOT
        $(error Please set GLFW_ROOT in makefile.local.env: "GLFW_ROOT := C:/msys64/clang64")
    endif
endif

# Compile with all warnings and c++17 in mind
CXXFLAGS = -Wall -std=c++17 -MMD -MP -Iinclude -I"$(GLFW_ROOT)/include" -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
LDFLAGS = -L"$(GLFW_ROOT)/lib"

CFLAGS = -Wall -std=c17 -MMD -MP -Iinclude -I"$(GLFW_ROOT)/include"

# External Link
EXLINKS = -lglfw3 -lopengl32 -lgdi32 -luser32 -lshell32 -lwinmm -lassimp

# Just grab every cpp file in src

SRC_IMGUI_BACKENDS = $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp \
                      $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SRC_IMGUI = $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp \
             $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp \
             $(IMGUI_DIR)/imgui_widgets.cpp
OBJ_IMGUI_BACKENDS = $(patsubst $(IMGUI_DIR)/backends/%.cpp, ./obj/backend_%.o, $(SRC_IMGUI_BACKENDS))
OBJ_IMGUI = $(patsubst $(IMGUI_DIR)/%.cpp, ./obj/imgui_%.o, $(SRC_IMGUI))
SRC_CPP = $(wildcard ./src/*.cpp) 
SRC_CPP += 
SRC_C = include/glad.c
# Object files are routed to obj directory
OBJ_CPP = $(patsubst ./src/%.cpp, ./obj/%.o, $(SRC_CPP))
OBJ_C = $(patsubst include/%.c, ./obj/%.o, $(SRC_C))
OBJ = $(OBJ_CPP) $(OBJ_IMGUI) $(OBJ_IMGUI_BACKENDS) $(OBJ_C)
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

./obj/imgui_%.o: $(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

./obj/backend_%.o: $(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


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
