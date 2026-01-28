#ifndef IMG_H
#define IMG_H
#include <stdint.h>
#include <string.h>

typedef struct {
    int width;
    int height;
    const uint16_t* data;
} Image;

extern const uint16_t kiet_start[8000];
extern const uint16_t minh_start[8000];
extern const uint16_t kiet_lose[67200];
extern const uint16_t minh_lose[67200];
extern Image kiet_start_image;
extern Image minh_start_image;
extern Image kiet_lose_image;
extern Image minh_lose_image;
#endif