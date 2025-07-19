# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall

# Libraries
LIBS = -lsfml-audio -lsfml-system

# Source and executable
SRCS = main.cpp
EXEC = terminal_rain

# --- Installation Paths ---
# The prefix where software is installed. Default is /usr/local
PREFIX ?= /usr/local
# Where the executable will go
BINDIR = $(PREFIX)/bin
# Where data files (like sounds) will go
DATADIR = $(PREFIX)/share/$(EXEC)

# Pass the data directory path to the C++ compiler
# The -D flag creates a macro, e.g. #define DATA_DIR "/usr/local/share/terminal_rain"
CXXFLAGS += -DDATA_DIR='"$(DATADIR)"'

# Default target
all: $(EXEC)

# Compilation rule
$(EXEC): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(SRCS) $(LIBS)

# --- Installation and Uninstallation ---

# 'install' target: copies files to system directories
install: all
	@echo "Installing $(EXEC) to $(BINDIR)..."
	@mkdir -p $(BINDIR)
	@install -m 755 $(EXEC) $(BINDIR)
	@echo "Installing data files to $(DATADIR)..."
	@mkdir -p $(DATADIR)
	@cp -r sounds $(DATADIR)/
	@echo "Installation complete. Run '$(EXEC)' from anywhere."

# 'uninstall' target: removes the installed files
uninstall:
	@echo "Removing $(EXEC) from $(BINDIR)..."
	@rm -f $(BINDIR)/$(EXEC)
	@echo "Removing data files from $(DATADIR)..."
	@rm -rf $(DATADIR)
	@echo "Uninstallation complete."

# 'clean' target: removes build files
clean:
	@rm -f $(EXEC)
