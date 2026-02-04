#include "def.h"
#include <freetype/ftbitmap.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttypes.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define DEFAULT_FONT
#define DEFAULT_FONT_SIZE 48
#define DEFAULT_WIDTH 1920
#define DEFAULT_HEIGHT 1080 
#define DEFAULT_BACKGROUND_COLOR 255, 255, 255
#define DEFAULT_TEXT_COLOR 0, 0, 0

typedef unsigned char color[3];

typedef struct {
  const char *font_path;
  const char *out_path;
  int font_size;
  int image_w, image_h;
  color bg_color;
  color txt_color;
} Options;

int render(const char *text, Options options) {
  FT_Library ft;
  FT_Face face;

  // Init FreeType
  if(FT_Init_FreeType(&ft)) {
    PERROR("Failed to initialise freetype.\n");
    return 1;
  }

  // Load font
  if(FT_New_Face(ft, options.font_path, 0, &face)) {
    PERROR("Failed to load font %s\n", options.font_path);
    FT_Done_FreeType(ft);
    return 1;
  }

  if(FT_Set_Pixel_Sizes(face, 0, options.font_size)) {
    PERROR("Failed to set font size to %d.\n", options.font_size);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return 1;
  }

  // Allocate image
  unsigned char *buf = (unsigned char *)malloc(options.image_w * options.image_h * 3);
  if(!buf) {
    PERROR("Failed to allocate image buffer.\n");
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return 1;
  }

  // Set background color
  for(int i = 0; i < options.image_w * options.image_h; i++) {
    buf[i * 3 + 0] = options.bg_color[0];
    buf[i * 3 + 1] = options.bg_color[1];
    buf[i * 3 + 2] = options.bg_color[2];
  }

  // Calculate total height and width of text rendered
  int total_height = 0;
  int total_width = 0;
  int max_line_len = 0;
  int cur_line_len = 0;
  for(int i = 0; text[i]; i++) {
    if(text[i] == '\n') {
      total_height += (face->size->metrics.height >> 6);
      if(cur_line_len > max_line_len) max_line_len = cur_line_len;
      cur_line_len = 0;
    } else {
      cur_line_len++;
    }
  }
  total_width = max_line_len * options.font_size / 2;

  // Calculate starting x and y positions
  // int y_start = (options.image_h - total_height) / 2;
  int y_start = (options.image_h - total_height) / 2 + (face->size->metrics.ascender >> 6);
  int x_start = (options.image_w - total_width) / 2;

  char *text_copy = strdup(text);
  if(!text_copy) {
    PERROR("strdup() failed.\n");
    free(buf);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return 1;
  }

  // Render text
  int y = y_start;
  int x = x_start;
  char *line = strtok(text_copy, "\n");
  while(line) {
    for(int i = 0; line[i] != '\0'; i++) {
      FT_UInt glyph_index = FT_Get_Char_Index(face, line[i]);

      if(FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING)) {
        continue;
      }

      FT_Bitmap *bitmap = &face->glyph->bitmap;
      int glyph_baseline_y = y + bitmap->rows - face->glyph->bitmap_top;

      // Write character to image
      for(int row = 0; row < bitmap->rows; row++) {
        for(int col = 0; col < bitmap->width; col++) {
          unsigned char pixel_value = bitmap->buffer[row * bitmap->width+ col];
          if(pixel_value) {
            int pixel_x =  x + col;
            int pixel_y = (glyph_baseline_y - (bitmap->rows - 1 - row));
            if(pixel_x >= options.image_w || pixel_x < 0) continue;
            if(pixel_y >= options.image_h || pixel_y < 0) continue;

            int idx = (pixel_y * options.image_w + pixel_x) * 3;

            if(idx + 2 >= options.image_w * options.image_h * 3 || idx < 0) continue;
            buf[idx + 0] = (options.txt_color[0] * pixel_value + buf[idx + 0] * (255 - pixel_value)) / 255;
            buf[idx + 1] = (options.txt_color[1] * pixel_value + buf[idx + 1] * (255 - pixel_value)) / 255;
            buf[idx + 2] = (options.txt_color[2] * pixel_value + buf[idx + 2] * (255 - pixel_value)) / 255;
          }
        }
      }

      x += face->glyph->advance.x >> 6;
    }

    // Next line
    y += options.font_size;
    x = x_start;
    line = strtok(NULL, "\n");
  }

  // Save image
  stbi_write_png(options.out_path, options.image_w, options.image_h, 3, buf, options.image_w * 3);

  // Cleanup
  free(buf);
  free(text_copy);
  FT_Done_Face(face);
  FT_Done_FreeType(ft);

  return 0;
}

int parse_options(Options *out, int argc, char *argv[]) {
  if(!out) return 1;

  struct option long_options[] = {
    {"font", required_argument, NULL, 'f'},
    {"bg_color", required_argument, NULL, 'b'},
    {"text_color", required_argument, NULL, 't'},
    {"size", required_argument, NULL, 's'},
    {"width", required_argument, NULL, 'w'},
    {"height", required_argument, NULL, 'h'},
    {"output", required_argument, NULL, 'o'},
    {"help", no_argument, NULL, 1},
    {NULL, 0, NULL, 0}};

  int opt;
  while((opt = getopt_long(argc, argv, "f:b:t:s:w:h:o:", long_options, NULL)) != -1) {
    switch(opt) {
      case 'f':
        out->font_path = optarg;
        break;
      case 'o':
        out->out_path = optarg;
        break;
      case 'b':
        sscanf(optarg, "#%2hhx%2hhx%2hhx", &out->bg_color[0], &out->bg_color[1], &out->bg_color[2]);
        break;
      case 't':
        sscanf(optarg, "#%2hhx%2hhx%2hhx", &out->txt_color[0], &out->txt_color[1], &out->txt_color[2]);
        break;
      case 's':
        out->font_size = atoi(optarg);
        break;
      case 'w':
        out->image_w = atoi(optarg);
        break;
      case 'h':
        out->image_h = atoi(optarg);
        break;
      case 1:
        return 2;
        break;
      default:
        return 1;
    }
  }

  return 0;
}

char *get_text(void) {
  size_t alloced = 1024;
  char *input_text = malloc(sizeof(char) * alloced);
  printf("%p\n", (void *)input_text);
  if(!input_text) {
    PERROR("malloc() failed.\n");
    return NULL;
  }
  size_t written = 0;

  char c;
  while((c = getchar()) && c != EOF) {
    if(written + 1 >= alloced) {
      printf("RESIZEING\n");
      alloced *= 2;
      char *new = realloc(input_text, sizeof(char) * alloced);
      if(!new) {
        PERROR("realloc() failed.\n");
        free(input_text);
        return NULL;
      }
    }
    input_text[written++] = c;
  }
  if(written = 0) {
    free(input_text);
    return NULL;
  }
  input_text[written] = '\0';
  return input_text;
}

void print_help(void) {
  printf("Usage: textwal [OPTIONS] < input_text.txt\n");
  printf("\nOptions:\n");
  printf("  -f, --font <path>           Path to the font file (e.g., /path/to/font.ttf)\n");
  printf("  -b, --bg_color <color>      Background color in hex (e.g., '#FFFFFF' for white)\n");
  printf("  -t, --text_color <color>    Text color in hex (e.g., '#000000' for black)\n");
  printf("  -s, --size <size>           Font size (default: %d)\n", DEFAULT_FONT_SIZE);
  printf("  -w, --width <width>         Image width (default: %d)\n", DEFAULT_WIDTH);
  printf("  -h, --height <height>       Image height (default: %d)\n", DEFAULT_HEIGHT);
  printf("  --help                      Show this help message\n");
}

int main(int argc, char *argv[]) {
  Options options = (Options) {
    .font_path = "/usr/share/fonts/liberation/LiberationSerif-Regular.ttf",
    .out_path = "output.png",
    .font_size = DEFAULT_FONT_SIZE,
    .image_w = DEFAULT_WIDTH,
    .image_h = DEFAULT_HEIGHT,
    .bg_color = {DEFAULT_BACKGROUND_COLOR},
    .txt_color = {DEFAULT_TEXT_COLOR}
  };

  switch(parse_options(&options, argc, argv)) {
    case 1:
      PERROR("Failed to parse options.\n");
      return 1;
    case 2:
      print_help();
      return 0;
    default: break;
  }

  // Get text and render
  char *input_text = get_text();
  printf("TEXT TO RENDER:\n%s\n", input_text);
  if(!input_text) {
    PERROR("Failed to get input text from stdin.\n");
    return 1;
  }

  if(render(input_text, options)) {
    PERROR("Failed to render image.\n");
    free(input_text);
    return 1;
  }

  printf("Image saved as %s\n", options.out_path);
  free(input_text);
  return 0;
}
