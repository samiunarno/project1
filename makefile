# 医院床位管理系统通用 Makefile
# 一键编译并运行
# 适用平台：Mac, iPad (iSH), Windows (MSYS2/Cygwin/WSL), Linux, GitHub Codespaces

# 强制 Make 使用 bash，以确保 echo -e 和颜色在 Codespaces/Ubuntu 中完美显示
SHELL := /bin/bash

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I.
TARGET = hospital
SRCS = A.c B.c C.c D.c utils.c main.c
OBJS = $(SRCS:.c=.o)

# 检测操作系统
UNAME_S := $(shell uname -s 2>/dev/null || echo "Windows")
UNAME_P := $(shell uname -p 2>/dev/null || echo "unknown")

# 特定平台设置
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

# Windows 检测
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

# Windows 原生
ifeq ($(OS),Windows_NT)
    RM = del /Q 2>nul
    EXEC_EXT = .exe
    CLEAR = cls
endif

# iPad (iSH) 检测
ifeq ($(UNAME_S), Linux)
    ifeq ($(UNAME_P), unknown)
        RM = rm -f
        EXEC_EXT = 
        CLEAR = clear
    endif
endif

# 默认值
ifeq ($(RM),)
    RM = rm -f
endif

ifeq ($(CLEAR),)
    CLEAR = clear
endif

TARGET_EXEC = $(TARGET)$(EXEC_EXT)

# 终端颜色配置
GREEN  = \033[0;32m
RED    = \033[0;31m
YELLOW = \033[0;33m
BLUE   = \033[0;34m
CYAN   = \033[0;36m
NC     = \033[0m

# 默认目标 - 仅编译
all: $(TARGET_EXEC)
	@echo -e "$(GREEN)✓ 编译成功！$(NC)"
	@echo -e "$(YELLOW)───────────────────────────────────────────────────────$(NC)"
	@echo -e "$(BLUE)运行指令：$(NC) ./$(TARGET_EXEC)"
	@echo -e "$(YELLOW)───────────────────────────────────────────────────────$(NC)"

# 编译并运行（先清屏）
run: $(TARGET_EXEC)
	@$(CLEAR)
	@echo -e "$(CYAN)╔════════════════════════════════════════════════════════════╗$(NC)"
	@echo -e "$(CYAN)║                 医院床位管理系统 - 正在运行                ║$(NC)"
	@echo -e "$(CYAN)╚════════════════════════════════════════════════════════════╝$(NC)"
	@echo ""
	./$(TARGET_EXEC)

# 编译并运行（不清屏）
quick: $(TARGET_EXEC)
	@echo -e "$(GREEN)🚀 正在启动程序...$(NC)"
	@echo -e "$(YELLOW)───────────────────────────────────────────────────────$(NC)"
	./$(TARGET_EXEC)

# 一键编译并运行（最常用）
allrun: $(TARGET_EXEC)
	@echo -e "$(GREEN)🔨 正在编译并运行...$(NC)"
	@echo -e "$(YELLOW)───────────────────────────────────────────────────────$(NC)"
	@$(CLEAR)
	./$(TARGET_EXEC)

# 生成可执行文件
$(TARGET_EXEC): $(OBJS)
	@echo -e "$(BLUE)🔗 正在链接...$(NC)"
	$(CC) $(CFLAGS) -o $@ $^
	@echo -e "$(GREEN)✓ 可执行文件已就绪： $(TARGET_EXEC)$(NC)"

# 编译目标文件
%.o: %.c
	@echo -e "$(BLUE)📦 正在编译 $<...$(NC)"
	$(CC) $(CFLAGS) -c $< -o $@

# Debug 模式编译并运行
debug: CFLAGS += -g -DDEBUG -O0
debug: clean allrun
	@echo -e "$(YELLOW)🔧 已生成包含符号表的 Debug 版本$(NC)"

# Release 模式编译并运行
release: CFLAGS += -O3 -DNDEBUG -march=native
release: clean allrun
	@echo -e "$(GREEN)⚡ 已生成优化后的 Release 版本$(NC)"

# 清理编译生成的文件
clean:
	@echo -e "$(YELLOW)🧹 正在清理...$(NC)"
	$(RM) $(OBJS) $(TARGET_EXEC) 2>nul || true
	$(RM) *.o 2>nul || true
	@echo -e "$(GREEN)✓ 清理完成$(NC)"

# 清理数据库文件
clean-db:
	@echo -e "$(YELLOW)🗄️  正在清理数据库...$(NC)"
	$(RM) departments_db.txt wards_db.txt beds_db.txt patients_db.txt 2>nul || true
	$(RM) backup_*.txt hospital_report.* 2>nul || true
	@echo -e "$(GREEN)✓ 数据库已清理$(NC)"

# 清理所有文件（编译文件 + 数据库）
cleanall: clean clean-db
	@echo -e "$(GREEN)✓ 完全清理完成$(NC)"

# 帮助菜单
help:
	@echo -e "$(BLUE)════════════════════════════════════════════════════════════════════$(NC)"
	@echo -e "$(GREEN)                     医院床位管理系统 - MAKEFILE$(NC)"
	@echo -e "$(BLUE)════════════════════════════════════════════════════════════════════$(NC)"
	@echo ""
	@echo -e "$(YELLOW)🔨 编译与运行命令：$(NC)"
	@echo -e "  $(GREEN)make run$(NC)        - 编译并运行（清屏） ⭐ 最常用"
	@echo -e "  $(GREEN)make allrun$(NC)     - 编译并运行（清屏）"
	@echo -e "  $(GREEN)make quick$(NC)      - 编译并运行（不清屏）"
	@echo -e "  $(GREEN)make all$(NC)        - 仅编译"
	@echo ""
	@echo -e "$(YELLOW)⚙️  高级编译：$(NC)"
	@echo -e "  $(GREEN)make debug$(NC)      - 使用 Debug 模式编译并运行（含调试符号）"
	@echo -e "  $(GREEN)make release$(NC)    - 使用 Release 模式编译并运行（优化性能）"
	@echo ""
	@echo -e "$(YELLOW)🧹 清理命令：$(NC)"
	@echo -e "  $(GREEN)make clean$(NC)      - 仅删除编译生成的文件"
	@echo -e "  $(GREEN)make clean-db$(NC)   - 删除数据库 TXT 文件"
	@echo -e "  $(GREEN)make cleanall$(NC)   - 删除所有文件（编译文件 + 数据库）"
	@echo ""
	@echo -e "$(YELLOW)💡 快速上手：$(NC)"
	@echo -e "  $(GREEN)make run$(NC)        - 最常用的启动命令"
	@echo -e "  $(GREEN)make clean run$(NC)  - 清理、重新编译并运行"
	@echo ""
	@echo -e "$(BLUE)════════════════════════════════════════════════════════════════════$(NC)"
	@echo -e "$(CYAN)当前系统平台： $(UNAME_S)$(NC)"
	@echo -e "$(BLUE)════════════════════════════════════════════════════════════════════$(NC)"

# 一行命令：清理、重新编译并运行
rerun: clean allrun
	@echo -e "$(GREEN)✓ 已重新编译并重启$(NC)"

# 默认目标
.DEFAULT_GOAL := run

# 伪目标
.PHONY: all run quick allrun debug release clean clean-db cleanall help rerun