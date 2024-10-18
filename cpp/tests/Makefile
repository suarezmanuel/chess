# Define variables
SRC = cpp/tests/test.cpp    # List of source files
EXECUTABLE = cpp/tests/test                   # Name of the executable
CXX = g++                           # Compiler
CXXFLAGS = -Wall -g
LDFLAGS = -lgtest -lgtest_main -pthread

# Declare phony targets
.PHONY: all run clean rebuild test FORCE

# Default target: build the executable
all: $(EXECUTABLE)

# Define FORCE as a phony target that is always out-of-date
FORCE:

# Build the executable (always rebuild because it depends on FORCE)
$(EXECUTABLE): FORCE
	$(CXX) $(CXXFLAGS) $(SRC) -o $(EXECUTABLE) $(LDFLAGS) 

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
	./$(EXECUTABLE)  # Replace with actual test command
