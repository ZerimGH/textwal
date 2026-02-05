SRCS = ./src/*.c
HDRS = ./src/*.h
OUT_DIR = ./build
OUT = $(OUT_DIR)/textwal

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -O3 
CINCLUDES = -I./freetype/include -I ./stb

INSTALL_DIR=/usr/local/bin

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(OUT): $(SRCS) $(HDRS) $(OUT_DIR)
	$(CC) $(SRCS) -o $(OUT) $(CFLAGS) $(CINCLUDES) -lfreetype -lm

all: $(OUT)

install: $(OUT)
	sudo cp $(OUT) $(INSTALL_DIR)/textwal

clean:
	rm -rf $(OUT_DIR)

.PHONY: all run clean

