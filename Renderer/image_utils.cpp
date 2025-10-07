
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "stb_image.h"
#include "image_utils.h"


extern "C" {

    DXE_API unsigned char* stb_LoadImage(const char* filename, int* x, int* y, int* channels, int desiredChannels) {
        return stbi_load(filename, x, y, channels, desiredChannels);
    }

    DXE_API void stb_FreeImage(unsigned char* data) {
        stbi_image_free(data);
    }

}