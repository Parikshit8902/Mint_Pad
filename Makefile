# The compiler to use
CXX = g++

# Name of your final executable (can be named anything, but then make sure to run it using that name. For example, if named "my_ide", then run as "./my_ide"
TARGET = mint_pad

# Compiler flags from pkg-config (for finding headers)
CXXFLAGS = $(shell pkg-config --cflags gtkmm-3.0 gtksourceviewmm-3.0)

# Linker flags from pkg-config (for linking libraries)
LDFLAGS = $(shell pkg-config --libs gtkmm-3.0 gtksourceviewmm-3.0)

# Source file
SRCS = main.cpp

# Default rule
all: $(TARGET)

# Rule for building the target executable
$(TARGET): $(SRCS)
	$(CXX) $(SRCS) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

# Rule for cleaning up compiled files
clean:
	rm -f $(TARGET)
