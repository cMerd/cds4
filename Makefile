CXX := g++
CXXFLAGS := -Wall -lraylib -I./inc/ --std=c++20
SRCDIR := src
BUILDDIR := build

# List of source files
SRCS := $(wildcard $(SRCDIR)/*.cpp)

# Generate object file names from source file names
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS))

# Default target for Linux
$(BUILDDIR)/cds4: $(OBJS)
	$(CXX) $(CXXFLAGS) -lm $^ -o $@

# Rule to compile each source file into object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Move the binary to path (/usr/bin)
install:
	sudo ln -s $(shell pwd)/build/cds4 /bin/cds4
	sudo cp ./assets/cds4.desktop /usr/share/applications/cds4.desktop

# Remove the binary from path (/usr/bin)
uninstall:
	sudo rm -f /bin/cds4
	sudo rm -f /usr/share/applications/cds4.desktop

# Clean target to remove all files in build directory
clean:
	rm -f $(BUILDDIR)/*


