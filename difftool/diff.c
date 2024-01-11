#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <getopt.h>

/* Support only yuv420p10le */

const int16_t value_offset = 512;
const uint16_t value_mask = 0x03ff;

/* In 10bit diff d = a - b */
uint16_t diff(uint16_t a, uint16_t b) {
  return ((int16_t)a - (int16_t)b + value_offset) & value_mask;
}

/* In 10bit add a = b + d */
uint16_t add(uint16_t a, uint16_t b) {
  return ((int16_t)a + (int16_t)b - value_offset) & value_mask;
}

void test_diff_and_merge() {
  for (uint16_t a = 0; a < 1024; ++a) {
    for (uint16_t b = 0; b < 1024; ++b) {
      uint16_t d = diff(a, b);
      uint16_t e = add(b, d);
      if (a != e) {
        fprintf(stderr, "d != e when a = %d, b = %d. d = %d, e = %d\n", a, b, d, e);
        return;
      }
    }
  }
}

int generate_difference_image(const char *input_a_filename, const char *input_b_filename, const char *output_filename, const int width, const int height) {
  FILE *fpa = fopen(input_a_filename, "rb");
  FILE *fpb = fopen(input_b_filename, "rb");
  FILE *fpout = fopen(output_filename, "wb");
  if (fpa == NULL || fpb == NULL || fpout == NULL) {
    fprintf(stderr, "Error in opening file.\n");
    return 1;
  }
  int total_value_count_per_frame = width * height + width * height / 4 + width * height / 4;
  uint16_t pixel_value_a = 0;
  uint16_t pixel_value_b = 0;
  uint16_t pixel_value_out = 0;
  for (int i = 0; i < total_value_count_per_frame; ++i)
  {
    fread(&pixel_value_a, sizeof(pixel_value_a), 1, fpa);
    fread(&pixel_value_b, sizeof(pixel_value_b), 1, fpb);
    pixel_value_out = diff(pixel_value_a, pixel_value_b);
    fwrite(&pixel_value_out, sizeof(pixel_value_out), 1, fpout);
  }
}

int recover_image(const char *input_b_filename, const char *input_d_filename, const char *output_filename, int width, int height) {
  FILE *fpb = fopen(input_b_filename, "rb");
  FILE *fpd = fopen(input_d_filename, "rb");
  FILE *fpout = fopen(output_filename, "wb");
  if (fpb == NULL || fpd == NULL || fpout == NULL) {
    fprintf(stderr, "Error in opening file.\n");
    return 1;
  }
  int total_value_count_per_frame = width * height + width * height / 4 + width * height / 4;
  uint16_t pixel_value_b = 0;
  uint16_t pixel_value_d = 0;
  uint16_t pixel_value_out = 0;
  for (int i = 0; i < total_value_count_per_frame; ++i)
  {
    fread(&pixel_value_b, sizeof(pixel_value_b), 1, fpb);
    fread(&pixel_value_d, sizeof(pixel_value_d), 1, fpd);
    pixel_value_out = add(pixel_value_b, pixel_value_d);
    fwrite(&pixel_value_out, sizeof(pixel_value_out), 1, fpout);
  }
}

void print_usege(FILE *output, char* const argv[]) {
  fprintf(output, "Usages\n");
  fprintf(output, "Generate difference image     : %s -g -a INPUT_A -b INPUT_B -o OUTPUT_DIFFERENCE_IMAGE [-w IMAGE_WIDTH] [-h IMAGE_WIDTH]\n", argv[0]);
  fprintf(output, "Recover from difference image : %s -r -b INPUT_B -d INPUT_DIFFERENCE_IMAGE -o OUTPUT_RECOVERED_IMAGE [-w IMAGE_WIDTH] [-h IMAGE_WIDTH]\n", argv[0]);
  fprintf(output, "\n");
  fprintf(output, "Pixel value calculation\n");
  fprintf(output, "  Generate : d = (a - b + offset) & mask\n");
  fprintf(output, "  Recover  : a = (b + d - offset) & mask\n");
  fprintf(output, "\n");
  fprintf(output, "Limitations\n");
  fprintf(output, "  * This program supports only yuv420p10le pixel format.\n");
  fprintf(output, "\n");
  fprintf(output, "Options\n\n");
  fprintf(output, "Mode selection\n");
  fprintf(output, "  -d : Specify generating mode.\n");
  fprintf(output, "  -r : Specify recovering mode.\n");
  fprintf(output, "\n");
  fprintf(output, "Input and Output\n");
  fprintf(output, "  -a : Specify input A RAW yuv image filename.\n");
  fprintf(output, "  -b : Specify input B RAW yuv image filename.\n");
  fprintf(output, "  -d : Specify input difference yuv image filename.\n");
  fprintf(output, "  -o : Specify output image filename.\n");
  fprintf(output, "  -w : Specify image width. (default : 3840)\n");
  fprintf(output, "  -h : Specify image height. (default : 2160)\n");
}

int main(int argc, char* const argv[]) {
  if (argc == 1) {
    print_usege(stderr, argv);
    return 1;
  }
  char *input_a_filename = NULL;
  char *input_b_filename = NULL;
  char *input_d_filename = NULL;
  char *output_filename = NULL;
  bool generating_mode = false;
  bool recovering_mode = false;
  int width = 3840;
  int height = 2160;
  
  const char optstring[] = "gra:b:d:o:w:h";
  struct option longopts[] = {
    { 0, 0, 0, 0 },
  };
  int c;
  while ((c = getopt_long(argc, argv, optstring, longopts, NULL)) != -1) {
    switch (c)
    {
    case 'g':
      generating_mode = true;
      break;
    case 'r':
      recovering_mode = true;
      break;
    case 'a':
      input_a_filename = optarg;
      break;
    case 'b':
      input_b_filename = optarg;
      break;
    case 'd':
      input_d_filename = optarg;
      break;
    case 'o':
      output_filename = optarg;
      break;
    case 'w':
      width = atoi(optarg);
      break;
    case 'h':
      height = atoi(optarg);
      break;

    case '?':
      fprintf(stderr, "ERROR : Some error occurred in parsing options.\n\n");
      print_usege(stderr, argv);
      return 1;
    default:
      printf("?? getopt returned character code 0%o ??\n", c);
      print_usege(stderr, argv);
      return 1;
    }
  }
  if (generating_mode == false && recovering_mode == false) {
    fprintf(stderr, "ERROR : No mode selection\n");
    return 1;
  }
  if (generating_mode == true && recovering_mode == true) {
    fprintf(stderr, "ERROR : Invalid mode selection\n");
    return 1;
  }
  bool error_flag = false;
  if (generating_mode)
  {
    if (input_a_filename == NULL) {
      fprintf(stderr, "ERROR : -a INPUT_A is not specified.\n");
      error_flag = true;
    }
    if (input_b_filename == NULL) {
      fprintf(stderr, "ERROR : -b INPUT_B is not specified.\n");
      error_flag = true;
    }
    if (output_filename == NULL) {
      fprintf(stderr, "ERROR : -o OUTPUT_DIFFERENCE_IMAGE is not specified.\n");
      error_flag = true;
    }
    if (error_flag) {
      return 1;
    }
    return generate_difference_image(input_a_filename, input_b_filename, output_filename, width, height);
  }
  else if (recovering_mode)
  {
    if (input_b_filename == NULL) {
      fprintf(stderr, "ERROR : -b INPUT_B is not specified.\n");
      error_flag = true;
    }
    if (input_d_filename == NULL) {
      fprintf(stderr, "ERROR : -d INPUT_DIFFERENCE_IMAGE is not specified.\n");
      error_flag = true;
    }
    if (output_filename == NULL) {
      fprintf(stderr, "ERROR : -o OUTPUT_DIFFERENCE_IMAGE is not specified.\n");
      error_flag = true;
    }
    if (error_flag) {
      return 1;
    }
    return recover_image(input_b_filename, input_d_filename, output_filename, width, height);
  }
  return 0;
}
