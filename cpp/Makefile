# Define variables
SRC = test.cpp    # List of source files
EXECUTABLE = cpp/test                   # Name of the executable
CXX = g++                           # Compiler
CXXFLAGS = -lgtest -lgtest_main -pthread -std=c++17

# Default target: build the executable
all: $(EXECUTABLE)

# Build the executable
$(EXECUTABLE): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(EXECUTABLE) $(SRC)

# Run the executable
.PHONY: run
run:
	./$(EXECUTABLE)

# Clean the executable
.PHONY: clean
clean:
	rm -f $(EXECUTABLE)

# Rebuild the project
.PHONY: rebuild
rebuild: clean all

# Test placeholder (if you want to integrate unit tests later)
.PHONY: test
test: all
	./$(EXECUTABLE) --test  # Replace with actual test command
