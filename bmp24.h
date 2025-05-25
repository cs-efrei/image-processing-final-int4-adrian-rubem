#ifndef BMP24_H
#define BMP24_H
#include <stdint.h>

// Image BMP 24 bits ===
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} t_pixel;

typedef struct {
    int width;
    int height;
    int colorDepth;
    t_pixel **data;
} t_bmp24;

// Allocation
t_pixel **bmp24_allocateDataPixels(int width, int height);
void bmp24_freeDataPixels(t_pixel **pixels, int height);
void bmp24_free(t_bmp24 *img);

// Save
t_bmp24 *bmp24_loadImage(const char *filename);
void bmp24_saveImage(t_bmp24 *img, const char *filename);

// Filters
void bmp24_negative(t_bmp24 *img);
void bmp24_grayscale(t_bmp24 *img);
void bmp24_brightness(t_bmp24 *img, int value);

// Convolution
void bmp24_applyFilter(t_bmp24 *img, float **kernel, int kernelSize);
void bmp24_boxBlur(t_bmp24 *img);
void bmp24_gaussianBlur(t_bmp24 *img);
void bmp24_outline(t_bmp24 *img);
void bmp24_emboss(t_bmp24 *img);
void bmp24_sharpen(t_bmp24 *img);

// P3
unsigned int *bmp24_computeHistogramR(const t_bmp24 *img);
unsigned int *bmp24_computeHistogramG(const t_bmp24 *img);
unsigned int *bmp24_computeHistogramB(const t_bmp24 *img);
void computeEqualizationLUT(unsigned int *hist, int totalPixels, uint8_t *lut);
void bmp24_equalize(t_bmp24 *img);

#endif // BMP24_H
