# Universal Makefile for Hospital Management System
# Compile and Run together with one command
# Works on: Mac, iPad (iSH), Windows (MSYS2/Cygwin/WSL), Linux, GitHub Codespaces

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I.
TARGET = hospital
SRCS = A.c B.c C.c D.c utils.c main.c
OBJS = $(SRCS:.c=.o)

# Detect operating system
UNAME_S := $(shell uname -s 2>/dev/null || echo "Windows")
UNAME_P := $(shell uname -p 2>/dev/null || echo "unknown")

# Platform-specific settings
ifeq ($(UNAME_S), Linux)
	RM = rm -f
	EXEC_EXT = 
	CLEAR = clear
endif

ifeq ($(UNAME_S), Darwin)
	RM = rm -f
	EXEC_EXT = 
	CLEAR = clear
endif

ifeq ($(UNAME_S), FreeBSD)
	RM = rm -f
	EXEC_EXT = 
	CLEAR = clear
endif

# Windows detection
ifneq ($(findstring MINGW, $(UNAME_S)),)
	RM = del /Q 2>nul || rm -f
	EXEC_EXT = .exe
	CLEAR = cls
endif

ifneq ($(findstring CYGWIN, $(UNAME_S)),)
	RM = rm -f
	EXEC_EXT = .exe
	CLEAR = clear
endif

ifneq ($(findstring MSYS, $(UNAME_S)),)
	RM = rm -f
	EXEC_EXT = .exe
	CLEAR = clear
endif

# Windows native
ifeq ($(OS),Windows_NT)
	RM = del /Q 2>nul
	EXEC_EXT = .exe
	CLEAR = cls
endif

# iPad (iSH) detection
ifeq ($(UNAME_S), Linux)
	ifeq ($(UNAME_P), unknown)
		RM = rm -f
		EXEC_EXT = 
		CLEAR = clear
	endif
endif

# Default values
ifeq ($(RM),)
	RM = rm -f
endif

ifeq ($(CLEAR),)
	CLEAR = clear
endif

TARGET_EXEC = $(TARGET)$(EXEC_EXT)

# Colors for terminal
GREEN  = \033[0;32m
RED    = \033[0;31m
YELLOW = \033[0;33m
BLUE   = \033[0;34m
CYAN   = \033[0;36m
NC     = \033[0m

# Default target - compile only
all: $(TARGET_EXEC)
	@echo "$(GREEN)✓ Build successful!$(NC)"
	@echo "$(YELLOW)───────────────────────────────────────────────────────$(NC)"
	@echo "$(BLUE)To run:$(NC) ./$(TARGET_EXEC)"
	@echo "$(YELLOW)───────────────────────────────────────────────────────$(NC)"

# Compile and run together (clears screen first)
run: $(TARGET_EXEC)
	@$(CLEAR)
	@echo "$(CYAN)╔════════════════════════════════════════════════════════════╗$(NC)"
	@echo "$(CYAN)║          HOSPITAL MANAGEMENT SYSTEM - RUNNING             ║$(NC)"
	@echo "$(CYAN)╚════════════════════════════════════════════════════════════╝$(NC)"
	@echo ""
	./$(TARGET_EXEC)

# Compile and run (no screen clear)
quick: $(TARGET_EXEC)
	@echo "$(GREEN)🚀 Starting program...$(NC)"
	@echo "$(YELLOW)───────────────────────────────────────────────────────$(NC)"
	./$(TARGET_EXEC)

# One-command build and run (most used)
allrun: $(TARGET_EXEC)
	@echo "$(GREEN)🔨 Compiling and running...$(NC)"
	@echo "$(YELLOW)───────────────────────────────────────────────────────$(NC)"
	@$(CLEAR)
	./$(TARGET_EXEC)

# Build the executable
$(TARGET_EXEC): $(OBJS)
	@echo "$(BLUE)🔗 Linking...$(NC)"
	$(CC) $(CFLAGS) -o $@ $^
	@echo "$(GREEN)✓ Executable ready: $(TARGET_EXEC)$(NC)"

# Compile object files
%.o: %.c
	@echo "$(BLUE)📦 Compiling $<...$(NC)"
	$(CC) $(CFLAGS) -c $< -o $@

# Debug build with run
debug: CFLAGS += -g -DDEBUG -O0
debug: clean allrun
	@echo "$(YELLOW)🔧 Debug build with symbols$(NC)"

# Release build with run
release: CFLAGS += -O3 -DNDEBUG -march=native
release: clean allrun
	@echo "$(GREEN)⚡ Release build with optimizations$(NC)"

# Clean build files
clean:
	@echo "$(YELLOW)🧹 Cleaning...$(NC)"
	$(RM) $(OBJS) $(TARGET_EXEC) 2>nul || true
	$(RM) *.o 2>nul || true
	@echo "$(GREEN)✓ Clean complete$(NC)"

# Clean database files
clean-db:
	@echo "$(YELLOW)🗄️  Cleaning database...$(NC)"
	$(RM) departments_db.txt wards_db.txt beds_db.txt patients_db.txt 2>nul || true
	$(RM) backup_*.txt hospital_report.* 2>nul || true
	@echo "$(GREEN)✓ Database cleaned$(NC)"

# Clean everything
cleanall: clean clean-db
	@echo "$(GREEN)✓ Full clean complete$(NC)"

# Help menu
help:
	@echo "$(BLUE)════════════════════════════════════════════════════════════════════$(NC)"
	@echo "$(GREEN)         HOSPITAL MANAGEMENT SYSTEM - MAKEFILE$(NC)"
	@echo "$(BLUE)════════════════════════════════════════════════════════════════════$(NC)"
	@echo ""
	@echo "$(YELLOW)🔨 BUILD & RUN COMMANDS:$(NC)"
	@echo "  $(GREEN)make run$(NC)        - Compile and run (clears screen) ⭐ MOST USED"
	@echo "  $(GREEN)make allrun$(NC)     - Compile and run (clears screen)"
	@echo "  $(GREEN)make quick$(NC)      - Compile and run (no screen clear)"
	@echo "  $(GREEN)make all$(NC)        - Compile only"
	@echo ""
	@echo "$(YELLOW)⚙️  ADVANCED BUILDS:$(NC)"
	@echo "  $(GREEN)make debug$(NC)      - Debug build with symbols and run"
	@echo "  $(GREEN)make release$(NC)    - Optimized release build and run"
	@echo ""
	@echo "$(YELLOW)🧹 CLEAN COMMANDS:$(NC)"
	@echo "  $(GREEN)make clean$(NC)      - Remove build files only"
	@echo "  $(GREEN)make clean-db$(NC)   - Remove database files"
	@echo "  $(GREEN)make cleanall$(NC)   - Remove everything"
	@echo ""
	@echo "$(YELLOW)💡 QUICK START:$(NC)"
	@echo "  $(GREEN)make run$(NC)        - Most common command"
	@echo "  $(GREEN)make clean run$(NC)  - Clean, rebuild, and run"
	@echo ""
	@echo "$(BLUE)════════════════════════════════════════════════════════════════════$(NC)"
	@echo "$(CYAN)Platform: $(UNAME_S)$(NC)"
	@echo "$(BLUE)════════════════════════════════════════════════════════════════════$(NC)"

# One-liner: clean, rebuild, and run
rerun: clean allrun
	@echo "$(GREEN)✓ Rebuilt and restarted$(NC)"

# Default target
.DEFAULT_GOAL := run

# Phony targets
.PHONY: all run quick allrun debug release clean clean-db cleanall help rerun