#ifndef BMP8_H
#define BMP8_H


// === The structure of the image BMP 8 bits ===
typedef struct {
    unsigned char header[54];
    unsigned char colorTable[1024];
    unsigned char *data;
    unsigned int width;
    unsigned int height;
    unsigned short colorDepth;
    unsigned int dataSize;
} t_bmp8;

// Based function
t_bmp8 *bmp8_loadImage(const char *filename);
void bmp8_saveImage(const char *filename, t_bmp8 *img);
void bmp8_free(t_bmp8 *img);
void bmp8_printInfo(const t_bmp8 *img);

// Simples filters
void bmp8_negative(t_bmp8 *img);
void bmp8_brightness(t_bmp8 *img, int value);
void bmp8_threshold(t_bmp8 *img, int threshold);

// Convolution filter
void bmp8_applyFilter(t_bmp8 *img, float **kernel, int kernelSize);

// Advanced filters
void bmp8_boxBlur(t_bmp8 *img);
void bmp8_gaussianBlur(t_bmp8 *img);
void bmp8_outline(t_bmp8 *img);
void bmp8_emboss(t_bmp8 *img);
void bmp8_sharpen(t_bmp8 *img);

// Part 3
unsigned int *bmp8_computeHistogram(t_bmp8 *img);
unsigned int *bmp8_computeCDF(unsigned int *hist);
void bmp8_equalize(t_bmp8 *img, unsigned int *cdf);
#endif // BMP8_H
