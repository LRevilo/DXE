#pragma once
#include "DXE.h"

extern "C" {
    DXE_API unsigned char* stb_LoadImage(const char* filename, int* x, int* y, int* channels, int desiredChannels);
    DXE_API void stb_FreeImage(unsigned char* data);
}