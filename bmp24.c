#include "bmp24.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


// Allocate a pixel matrix
t_pixel **bmp24_allocateDataPixels(int width, int height) {
    t_pixel **pixels = malloc(height * sizeof(t_pixel *));
    if (!pixels) return NULL;
    for (int i = 0; i < height; i++) {
        pixels[i] = malloc(width * sizeof(t_pixel));
        if (!pixels[i]) {
            for (int j = 0; j < i; j++) free(pixels[j]);
            free(pixels);
            return NULL;
        }
    }
    return pixels;
}

// Free memory for a pixel matrix
void bmp24_freeDataPixels(t_pixel **pixels, int height) {
    for (int i = 0; i < height; i++) free(pixels[i]);
    free(pixels);
}


// Free the BMP
void bmp24_free(t_bmp24 *img) {
    if (img) {
        bmp24_freeDataPixels(img->data, img->height);
        free(img);
    }
}

// Load a BMP 24 bit image
t_bmp24 *bmp24_loadImage(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("Erreur ouverture fichier %s\n", filename);
        return NULL;
    }

    // Manually read only the essential header fields
    uint16_t type;
    int32_t width, height;
    uint16_t bits;
    uint32_t compression, offset;

    fseek(f, 0, SEEK_SET);
    // BMP signature
    fread(&type, sizeof(uint16_t), 1, f);

    fseek(f, 18, SEEK_SET);
    fread(&width, sizeof(int32_t), 1, f);
    fread(&height, sizeof(int32_t), 1, f);

    fseek(f, 28, SEEK_SET);
    fread(&bits, sizeof(uint16_t), 1, f);

    fseek(f, 30, SEEK_SET);
    fread(&compression, sizeof(uint32_t), 1, f);

    fseek(f, 10, SEEK_SET);
    fread(&offset, sizeof(uint32_t), 1, f);

    // Validate BMP 24-bit uncompressed format
    if (type != 0x4D42 || bits != 24 || compression != 0) {
        printf("Incompatible file. BMP 24 bits must be uncompressed .\n");
        fclose(f);
        return NULL;
    }

    // structure is allocated
    t_bmp24 *img = malloc(sizeof(t_bmp24));
    img->width = width;
    img->height = height;
    img->colorDepth = bits;
    img->data = bmp24_allocateDataPixels(width, height);
    if (!img->data) {
        fclose(f);
        free(img);
        return NULL;
    }

    fseek(f, offset, SEEK_SET);
    int padding = (4 - (width * 3) % 4) % 4;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char bgr[3];
            fread(bgr, 1, 3, f);
            img->data[height - 1 - y][x].blue = bgr[0];
            img->data[height - 1 - y][x].green = bgr[1];
            img->data[height - 1 - y][x].red = bgr[2];
        }
        // padding byte skip
        fseek(f, padding, SEEK_CUR);
    }

    fclose(f);
    printf("Image loaded : %dx%d\n", width, height);
    return img;
}

// Save 24-bytes
void bmp24_saveImage(t_bmp24 *img, const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        printf("Writing error  %s\n", filename);
        return;
    }

    // // BMP header
    uint16_t type = 0x4D42;
    uint32_t offset = 54;
    uint32_t size = offset + (img->width * 3 + (4 - (img->width * 3 % 4)) % 4) * img->height;
    uint16_t reserved = 0;

    // BMP info header
    fwrite(&type, sizeof(uint16_t), 1, f);
    fwrite(&size, sizeof(uint32_t), 1, f);
    fwrite(&reserved, sizeof(uint16_t), 1, f);
    fwrite(&reserved, sizeof(uint16_t), 1, f);
    fwrite(&offset, sizeof(uint32_t), 1, f);

    // Header info
    uint32_t headerSize = 40;
    uint16_t planes = 1;
    uint16_t bits = 24;
    uint32_t compression = 0;
    uint32_t imageSize = size - offset;
    int32_t resolution = 2835;

    fwrite(&headerSize, sizeof(uint32_t), 1, f);
    fwrite(&img->width, sizeof(int32_t), 1, f);
    fwrite(&img->height, sizeof(int32_t), 1, f);
    fwrite(&planes, sizeof(uint16_t), 1, f);
    fwrite(&bits, sizeof(uint16_t), 1, f);
    fwrite(&compression, sizeof(uint32_t), 1, f);
    fwrite(&imageSize, sizeof(uint32_t), 1, f);
    fwrite(&resolution, sizeof(int32_t), 1, f);
    fwrite(&resolution, sizeof(int32_t), 1, f);
    // Ncolors = 0
    fwrite(&compression, sizeof(uint32_t), 1, f);
    // Important colors = 0
    fwrite(&compression, sizeof(uint32_t), 1, f);

    // Pixel is write
    int padding = (4 - (img->width * 3) % 4) % 4;
    unsigned char pad[3] = {0, 0, 0};

    for (int y = img->height - 1; y >= 0; y--) {
        for (int x = 0; x < img->width; x++) {
            unsigned char bgr[3] = {
                img->data[y][x].blue,
                img->data[y][x].green,
                img->data[y][x].red
            };
            fwrite(bgr, 1, 3, f);
        }
        fwrite(pad, 1, padding, f);
    }

    fclose(f);
    printf("Image save successfully in %s\n", filename);
}

// Color inverting
void bmp24_negative(t_bmp24 *img) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            t_pixel *p = &img->data[y][x];
            p->red = 255 - p->red;
            p->green = 255 - p->green;
            p->blue = 255 - p->blue;
        }
    }
}

// Grayscale converting
void bmp24_grayscale(t_bmp24 *img) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            t_pixel *p = &img->data[y][x];
            uint8_t g = (p->red + p->green + p->blue) / 3;
            p->red = p->green = p->blue = g;
        }
    }
}

// Adjust brightness
void bmp24_brightness(t_bmp24 *img, int value) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            t_pixel *p = &img->data[y][x];
            p->red = fminf(fmaxf(p->red + value, 0), 255);
            p->green = fminf(fmaxf(p->green + value, 0), 255);
            p->blue = fminf(fmaxf(p->blue + value, 0), 255);
        }
    }
}

// Generic convolution
t_pixel bmp24_convolution(t_bmp24 *img, int x, int y, float **kernel, int kernelSize) {
    int n = kernelSize / 2;
    float r = 0, g = 0, b = 0;

    for (int ky = -n; ky <= n; ky++) {
        for (int kx = -n; kx <= n; kx++) {
            int px = x + kx;
            int py = y + ky;
            if (px >= 0 && px < img->width && py >= 0 && py < img->height) {
                t_pixel p = img->data[py][px];
                float coeff = kernel[ky + n][kx + n];
                r += p.red * coeff;
                g += p.green * coeff;
                b += p.blue * coeff;
            }
        }
    }

    t_pixel result;
    result.red = (uint8_t)fminf(fmaxf(r, 0), 255);
    result.green = (uint8_t)fminf(fmaxf(g, 0), 255);
    result.blue = (uint8_t)fminf(fmaxf(b, 0), 255);
    return result;
}

void bmp24_applyFilter(t_bmp24 *img, float **kernel, int kernelSize) {
    t_pixel **newData = bmp24_allocateDataPixels(img->width, img->height);
    if (!newData) return;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            newData[y][x] = bmp24_convolution(img, x, y, kernel, kernelSize);
        }
    }

    bmp24_freeDataPixels(img->data, img->height);
    img->data = newData;
}

// Filters advanced
void bmp24_boxBlur(t_bmp24 *img) {
    float box[3][3] = {
        {1/9.f, 1/9.f, 1/9.f},
        {1/9.f, 1/9.f, 1/9.f},
        {1/9.f, 1/9.f, 1/9.f}
    };
    float* kernel[3] = { box[0], box[1], box[2] };
    bmp24_applyFilter(img, kernel, 3);
}

void bmp24_gaussianBlur(t_bmp24 *img) {
    float gauss[3][3] = {
        {1/16.f, 2/16.f, 1/16.f},
        {2/16.f, 4/16.f, 2/16.f},
        {1/16.f, 2/16.f, 1/16.f}
    };
    float* kernel[3] = { gauss[0], gauss[1], gauss[2] };
    bmp24_applyFilter(img, kernel, 3);
}

void bmp24_outline(t_bmp24 *img) {
    float outline[3][3] = {
        {-1, -1, -1},
        {-1,  8, -1},
        {-1, -1, -1}
    };
    float* kernel[3] = { outline[0], outline[1], outline[2] };
    bmp24_applyFilter(img, kernel, 3);
}

void bmp24_emboss(t_bmp24 *img) {
    float emboss[3][3] = {
        {-2, -1, 0},
        {-1,  1, 1},
        { 0,  1, 2}
    };
    float* kernel[3] = { emboss[0], emboss[1], emboss[2] };
    bmp24_applyFilter(img, kernel, 3);
}

void bmp24_sharpen(t_bmp24 *img) {
    float sharpen[3][3] = {
        { 0, -1,  0},
        {-1,  5, -1},
        { 0, -1,  0}
    };
    float* kernel[3] = { sharpen[0], sharpen[1], sharpen[2] };
    bmp24_applyFilter(img, kernel, 3);
}

// Channel red
unsigned int *bmp24_computeHistogramR(const t_bmp24 *img) {
    unsigned int *hist=calloc(256, sizeof(unsigned int)); // We use calloc(256, sizeof(unsigned int)) to create an array of 256 integers (0-255 colour levels).
    if (!hist) return 0;
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) { // Through each pixel
            uint8_t r = img->data[y][x].red; // Extract red value
            hist[r]++;
        }
    }
    return hist;
}

// Channel green
unsigned int *bmp24_computeHistogramG(const t_bmp24 *img) {
    unsigned int *hist=calloc(256, sizeof(unsigned int)); // We use calloc(256, sizeof(unsigned int)) also
    if (!hist) return 0;
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) { // Through each pixel
            uint8_t g = img->data[y][x].green; // Extract green value here
            hist[g]++;
        }
    }
    return hist;
}


// Channel blue
unsigned int *bmp24_computeHistogramB(const t_bmp24 *img) {
    unsigned int *hist=calloc(256, sizeof(unsigned int)); // And here finally
    if (!hist) return 0;
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) { // Through each pixel
            uint8_t b = img->data[y][x].blue; // Extract blue value
            hist[b]++;
        }
    }
    return hist;
}

void computeEqualizationLUT(unsigned int *hist, int total, uint8_t *lut) {
    unsigned int cdf[256] = {0};
    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i - 1] + hist[i];
    }

    unsigned int cdf_min = 0;
    for (int i = 0; i < 256; i++) {
        if (cdf[i] != 0) {
            cdf_min = cdf[i];
            break;
        }
    }

    for (int i = 0; i < 256; i++) {
        if (total - cdf_min != 0)
            lut[i] = (uint8_t)roundf(((float)(cdf[i] - cdf_min) / (total - cdf_min)) * 255);
        else
            lut[i] = 0;
    }
}

void bmp24_equalize(t_bmp24 *img) {
    int width = img->width;
    int height = img->height;
    int size = width * height;

    // Allocation buffer
    float *Y = malloc(size * sizeof(float));
    float *U = malloc(size * sizeof(float));
    float *V = malloc(size * sizeof(float));

    if (!Y || !U || !V) {
        printf("Memory allocation failed.\n");
        free(Y); free(U); free(V);
        return;
    }

    // 1 : conversion RGB to YUV
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            t_pixel p = img->data[y][x];
            int i = y * width + x;

            float r = p.red;
            float g = p.green;
            float b = p.blue;

            Y[i] = 0.299f * r + 0.587f * g + 0.114f * b;
            U[i] = -0.14713f * r - 0.28886f * g + 0.436f * b;
            V[i] =  0.615f * r - 0.51499f * g - 0.10001f * b;
        }
    }

    // 2 : compute the histogram and CDF on Y
    unsigned int hist[256] = {0};
    for (int i = 0; i < size; i++) {
        int y_val = (int)fminf(fmaxf(roundf(Y[i]), 0), 255);
        hist[y_val]++;
    }

    // CDF
    unsigned int cdf[256] = {0};
    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i - 1] + hist[i];
    }

    // LUT from remapping
    uint8_t map[256];
    for (int i = 0; i < 256; i++) {
        map[i] = (uint8_t)roundf(((float)(cdf[i] - cdf[0]) / (size - cdf[0])) * 255.0f);
    }

    // 3 : rebuild RGB from YUV
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int i = y * width + x;

            float y_eq = (float)map[(int)fminf(fmaxf(roundf(Y[i]), 0), 255)];
            float u = U[i];
            float v = V[i];

            float r = y_eq + 1.13983f * v;
            float g = y_eq - 0.39465f * u - 0.58060f * v;
            float b = y_eq + 2.03211f * u;

            img->data[y][x].red   = (uint8_t)fminf(fmaxf(r, 0), 255);
            img->data[y][x].green = (uint8_t)fminf(fmaxf(g, 0), 255);
            img->data[y][x].blue  = (uint8_t)fminf(fmaxf(b, 0), 255);
        }
    }

    // clean up
    free(Y); free(U); free(V);
    printf("Histogram Equalization (Y-channel) applied successfully.\n");
}

