SRCS = ./src/*.c
HDRS = ./src/*.h
OUT_DIR = ./build
OUT = $(OUT_DIR)/textwal

CC = gcc
CFLAGS = -Wall -Wextra -pedantic
CINCLUDES = -I./freetype/include -I ./stb

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(OUT): $(SRCS) $(HDRS) $(OUT_DIR)
	$(CC) $(SRCS) -o $(OUT) $(CFLAGS) $(CINCLUDES) -lfreetype

all: $(OUT)

run: $(OUT)
	$(OUT) -t '#FF0000' < example.txt
	feh output.png

clean:
	rm -rf $(OUT_DIR)

.PHONY: all run clean

