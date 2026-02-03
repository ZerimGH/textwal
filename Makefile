SRCS = ./src/*.c
HDRS = ./src/*.h
OUT_DIR = ./build
OUT = $(OUT_DIR)/textwal

CC = gcc
CFLAGS = -Wall -Wextra -pedantic
CINCLUDES = -I./freetype/include -I ./stb

FREETYPE_DIR = ./freetype
FREETYPE_LIB_DIR = $(FREETYPE_DIR)/build/lib
FREETYPE_INCLUDE_DIR = $(FREETYPE_DIR)/include
FREETYPE_LIBS = -lfreetype

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(OUT): $(SRCS) $(HDRS) $(OUT_DIR)
	$(CC) $(SRCS) -o $(OUT) $(CFLAGS) $(CINCLUDES) -L$(FREETYPE_LIB_DIR) $(FREETYPE_LIBS)

all: $(OUT)

run: $(OUT)
	echo 'Hello, world!' | $(OUT)
	feh output.png

clean:
	rm -rf $(OUT_DIR)

.PHONY: all run clean

