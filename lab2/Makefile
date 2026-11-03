CC = gcc
CFLAGS ?= -std=c11 -Wall -Wextra -O2

all: build/main

build/main: main.c bg_posix.c bg_win.c bg.h
	$(CC) $(CFLAGS) main.c bg_posix.c bg_win.c -o build/main
