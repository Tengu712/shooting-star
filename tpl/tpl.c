#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

unsigned char *image_file_to_bitmap(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels) {
    return stbi_load(filename, x, y, channels_in_file, desired_channels);
}
