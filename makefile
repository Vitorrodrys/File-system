# Compiler and Linker
CXX = g++           # C++ compiler (you can change this to clang++, etc.)
CXXFLAGS = -std=c++17 -Wall -g  # Compiler flags (for example: C++17, all warnings, debug symbols)
LDFLAGS =            # Linker flags, if needed (usually empty for simple cases)

# Directories
SRC_DIR = src        # Source files directory
OBJ_DIR = obj        # Object files directory
BIN_DIR = bin        # Output directory for the executable

# File Extensions
SRC_EXT = cpp        # Source file extension
OBJ_EXT = o          # Object file extension

# Sources and Objects
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)         # All source files in the src directory
OBJECTS = $(SOURCES:$(SRC_DIR)/%.$(SRC_EXT)=$(OBJ_DIR)/%.$(OBJ_EXT))  # Corresponding object files
EXEC = $(BIN_DIR)   # The final executable name

# Default target
all: $(EXEC)

# Rule for creating the executable
$(EXEC): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(EXEC) $(LDFLAGS)

# Rule for compiling .cpp files to .o files
$(OBJ_DIR)/%.$(OBJ_EXT): $(SRC_DIR)/%.$(SRC_EXT)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -rf $(OBJ_DIR)/*.o $(BIN_DIR)/$(EXEC)

# Phony targets (non-file targets)
.PHONY: all clean
