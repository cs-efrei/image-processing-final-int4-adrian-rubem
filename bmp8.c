#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp8.h"

t_bmp8* bmp8_loadImage(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return NULL;
    }

    t_bmp8* img = (t_bmp8*)malloc(sizeof(t_bmp8));
    if (!img) {
        fclose(file);
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    if (fread(img->header, 1, 54, file) != 54) {
        fclose(file);
        free(img);
        fprintf(stderr, "Error: Not a valid BMP file\n");
        return NULL;
    }

    if (img->header[0] != 'B' || img->header[1] != 'M') {
        fclose(file);
        free(img);
        fprintf(stderr, "Error: Not a valid BMP file\n");
        return NULL;
    }

    img->width = *(unsigned int*)&img->header[18];
    img->height = *(unsigned int*)&img->header[22];
    img->colorDepth = *(unsigned short*)&img->header[28];
    img->dataSize = *(unsigned int*)&img->header[34];

    if (img->colorDepth != 8) {
        fclose(file);
        free(img);
        fprintf(stderr, "Error: Not an 8-bit grayscale image\n");
        return NULL;
    }

    if (fread(img->colorTable, 1, 1024, file) != 1024) {
        fclose(file);
        free(img);
        fprintf(stderr, "Error: Could not read color table\n");
        return NULL;
    }

    img->data = (unsigned char*)malloc(img->dataSize);
    if (!img->data) {
        fclose(file);
        free(img);
        fprintf(stderr, "Error: Memory allocation for image data failed\n");
        return NULL;
    }

    if (fread(img->data, 1, img->dataSize, file) != img->dataSize) {
        fclose(file);
        free(img->data);
        free(img);
        fprintf(stderr, "Error: Could not read image data\n");
        return NULL;
    }

    fclose(file);
    return img;
}

void bmp8_saveImage(const char* filename, t_bmp8* img) {
    if (!img || !img->data) {
        fprintf(stderr, "Error: Invalid image data\n");
        return;
    }

    FILE* file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Error: Could not create file %s\n", filename);
        return;
    }

    if (fwrite(img->header, 1, 54, file) != 54) {
        fclose(file);
        fprintf(stderr, "Error: Could not write header\n");
        return;
    }

    if (fwrite(img->colorTable, 1, 1024, file) != 1024) {
        fclose(file);
        fprintf(stderr, "Error: Could not write color table\n");
        return;
    }

    if (fwrite(img->data, 1, img->dataSize, file) != img->dataSize) {
        fclose(file);
        fprintf(stderr, "Error: Could not write image data\n");
        return;
    }

    fclose(file);
}

void bmp8_free(t_bmp8* img) {
    if (img) {
        if (img->data) {
            free(img->data);
        }
        free(img);
    }
}

void bmp8_printInfo(t_bmp8* img) {
    if (!img) {
        printf("Error: Invalid image\n");
        return;
    }

    printf("Image Info:\n");
    printf("    Width: %u\n", img->width);
    printf("    Height: %u\n", img->height);
    printf("    Color Depth: %u\n", img->colorDepth);
    printf("    Data Size: %u\n", img->dataSize);
}

void bmp8_negative(t_bmp8* img) {
    if (!img || !img->data) return;

    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->data[i] = 255 - img->data[i];
    }
}

void bmp8_brightness(t_bmp8* img, int value) {
    if (!img || !img->data) return;

    for (unsigned int i = 0; i < img->dataSize; i++) {
        int new_value = img->data[i] + value;
        if (new_value > 255) new_value = 255;
        if (new_value < 0) new_value = 0;
        img->data[i] = (unsigned char)new_value;
    }
}

void bmp8_threshold(t_bmp8* img, int threshold) {
    if (!img || !img->data) return;
    if (threshold < 0) threshold = 0;
    if (threshold > 255) threshold = 255;

    for (unsigned int i = 0; i < img->dataSize; i++) {
        img->data[i] = (img->data[i] >= threshold) ? 255 : 0;
    }
}

void bmp8_applyFilter(t_bmp8* img, float** kernel, int kernelSize) {
    if (!img || !img->data || !kernel || kernelSize % 2 == 0) return;

    unsigned char* tempData = (unsigned char*)malloc(img->dataSize);
    if (!tempData) return;
    memcpy(tempData, img->data, img->dataSize);

    int n = kernelSize / 2;
    int width = img->width;
    int height = img->height;

    for (int y = n; y < height - n; y++) {
        for (int x = n; x < width - n; x++) {
            float sum = 0.0f;

            for (int ky = -n; ky <= n; ky++) {
                for (int kx = -n; kx <= n; kx++) {
                    int pixelX = x + kx;
                    int pixelY = y + ky;
                    unsigned char pixelValue = img->data[pixelY * width + pixelX];
                    sum += pixelValue * kernel[ky + n][kx + n];
                }
            }

            if (sum > 255) sum = 255;
            if (sum < 0) sum = 0;
            tempData[y * width + x] = (unsigned char)sum;
        }
    }

    memcpy(img->data, tempData, img->dataSize);
    free(tempData);
}