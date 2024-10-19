SRC = test.cpp                     # Source file
EXECUTABLE = ./bin/test        # Full path to the executable
CXX = g++                          # Compiler
CXXFLAGS = -Wall -g                # Compiler flags
LDFLAGS = -lgtest -lgtest_main -pthread  # Linker flags for Google Test

# Declare all phony targets in a single line to prevent overrides
.PHONY: all run clean rebuild test FORCE

# Default target: build the executable
all: $(EXECUTABLE)

# Define FORCE as a phony target that is always out-of-date
FORCE:

# Rule to ensure BIN_DIR exists before building the executable
$(EXECUTABLE): FORCE | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(EXECUTABLE) $(LDFLAGS)

# Create BIN_DIR if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Run the executable
run: $(EXECUTABLE)
	./$(EXECUTABLE)
	g++ plot_records.cpp -o ./bin/plot_records
	./plot_records

# Clean the executable
clean:
	rm -f $(EXECUTABLE)

# Rebuild the project
rebuild: clean all

# Test target (runs the executable)
test: all
	./$(EXECUTABLE)  # Replace with actual test command if different
