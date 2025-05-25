#include "bmp24.h"
#include <string.h>
#include <math.h>

#define BITMAP_MAGIC    0x00
#define BITMAP_SIZE    0x02
#define BITMAP_OFFSET    0x0A
#define BITMAP_WIDTH   0x12
#define BITMAP_HEIGHT    0x16
#define BITMAP_DEPTH    0x1C
#define BITMAP_SIZE_RAW    0x22

#define BMP_TYPE    0x4D42
#define HEADER_SIZE    0x0E
#define INFO_SIZE    0x28
#define DEFAULT_DEPTH    0x18

static void file_rawRead(uint32_t position, void *buffer, uint32_t size, size_t n, FILE *file) {
    fseek(file, position, SEEK_SET);
    fread(buffer, size, n, file);
}

static void file_rawWrite(uint32_t position, void *buffer, uint32_t size, size_t n, FILE *file) {
    fseek(file, position, SEEK_SET);
    fwrite(buffer, size, n, file);
}

t_pixel **bmp24_allocateDataPixels(int width, int height) {
    t_pixel **pixels = (t_pixel **)malloc(height * sizeof(t_pixel *));
    if (!pixels) return NULL;

    for (int i = 0; i < height; i++) {
        pixels[i] = (t_pixel *)malloc(width * sizeof(t_pixel));
        if (!pixels[i]) {
            for (int j = 0; j < i; j++) {
                free(pixels[j]);
            }
            free(pixels);
            return NULL;
        }
    }

    return pixels;
}

void bmp24_freeDataPixels(t_pixel **pixels, int height) {
    if (!pixels) return;

    for (int i = 0; i < height; i++) {
        free(pixels[i]);
    }
    free(pixels);
}

t_bmp24 *bmp24_allocate(int width, int height, int colorDepth) {
    t_bmp24 *img = (t_bmp24 *)malloc(sizeof(t_bmp24));
    if (!img) return NULL;

    img->width = width;
    img->height = height;
    img->colorDepth = colorDepth;

    img->data = bmp24_allocateDataPixels(width, height);
    if (!img->data) {
        free(img);
        return NULL;
    }

    return img;
}

void bmp24_free(t_bmp24 *img) {
    if (img) {
        if (img->data) {
            bmp24_freeDataPixels(img->data, img->height);
        }
        free(img);
    }
}

void bmp24_readPixelValue(t_bmp24 *image, int x, int y, FILE *file) {
    if (!image || !file || x < 0 || x >= image->width || y < 0 || y >= image->height) return;

    int row = image->height - 1 - y;
    fread(&image->data[row][x], sizeof(t_pixel), 1, file);
}

void bmp24_readPixelData(t_bmp24 *image, FILE *file) {
    if (!image || !file) return;

    int padding = (4 - (image->width * 3) % 4) % 4;
    char pad[3];

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            bmp24_readPixelValue(image, x, y, file);
        }
        if (padding > 0) {
            fread(pad, 1, padding, file);
        }
    }
}

void bmp24_writePixelValue(t_bmp24 *image, int x, int y, FILE *file) {
    if (!image || !file || x < 0 || x >= image->width || y < 0 || y >= image->height) return;

    int row = image->height - 1 - y;
    fwrite(&image->data[row][x], sizeof(t_pixel), 1, file);
}

void bmp24_writePixelData(t_bmp24 *image, FILE *file) {
    if (!image || !file) return;

    int padding = (4 - (image->width * 3) % 4) % 4;
    char pad[3] = {0, 0, 0}; 

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            bmp24_writePixelValue(image, x, y, file);
        }
        if (padding > 0) {
            fwrite(pad, 1, padding, file);
        }
    }
}

t_bmp24 *bmp24_loadImage(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return NULL;
    }

    uint16_t magic;
    file_rawRead(BITMAP_MAGIC, &magic, sizeof(uint16_t), 1, file);
    if (magic != BMP_TYPE) {
        fclose(file);
        fprintf(stderr, "Error: Not a valid BMP file\n");
        return NULL;
    }

    int32_t width, height;
    file_rawRead(BITMAP_WIDTH, &width, sizeof(int32_t), 1, file);
    file_rawRead(BITMAP_HEIGHT, &height, sizeof(int32_t), 1, file);

    uint16_t depth;
    file_rawRead(BITMAP_DEPTH, &depth, sizeof(uint16_t), 1, file);
    if (depth != 24) {
        fclose(file);
        fprintf(stderr, "Error: Not a 24-bit color image\n");
        return NULL;
    }

    t_bmp24 *img = bmp24_allocate(width, height, depth);
    if (!img) {
        fclose(file);
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    file_rawRead(0, &img->header, sizeof(t_bmp_header), 1, file);
    file_rawRead(HEADER_SIZE, &img->header_info, sizeof(t_bmp_info), 1, file);

    fseek(file, img->header.offset, SEEK_SET);

    bmp24_readPixelData(img, file);

    fclose(file);
    return img;
}

void bmp24_saveImage(t_bmp24 *img, const char *filename) {
    if (!img) return;

    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Error: Could not create file %s\n", filename);
        return;
    }

    img->header_info.width = img->width;
    img->header_info.height = img->height;
    img->header_info.bits = img->colorDepth;

    int rowSize = (img->width * 3 + 3) & ~3;
    img->header_info.imagesize = rowSize * img->height;
    img->header.size = sizeof(t_bmp_header) + sizeof(t_bmp_info) + img->header_info.imagesize;

    file_rawWrite(0, &img->header, sizeof(t_bmp_header), 1, file);
    file_rawWrite(HEADER_SIZE, &img->header_info, sizeof(t_bmp_info), 1, file);

    fseek(file, img->header.offset, SEEK_SET);

    bmp24_writePixelData(img, file);

    fclose(file);
}

void bmp24_negative(t_bmp24 *img) {
    if (!img || !img->data) return;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            img->data[y][x].red = 255 - img->data[y][x].red;
            img->data[y][x].green = 255 - img->data[y][x].green;
            img->data[y][x].blue = 255 - img->data[y][x].blue;
        }
    }
}

void bmp24_grayscale(t_bmp24 *img) {
    if (!img || !img->data) return;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            uint8_t gray = (uint8_t)(0.299 * img->data[y][x].red +
                                    0.587 * img->data[y][x].green +
                                    0.114 * img->data[y][x].blue);
            img->data[y][x].red = gray;
            img->data[y][x].green = gray;
            img->data[y][x].blue = gray;
        }
    }
}

void bmp24_brightness(t_bmp24 *img, int value) {
    if (!img || !img->data) return;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int r = img->data[y][x].red + value;
            int g = img->data[y][x].green + value;
            int b = img->data[y][x].blue + value;

            img->data[y][x].red = (r > 255) ? 255 : (r < 0) ? 0 : r;
            img->data[y][x].green = (g > 255) ? 255 : (g < 0) ? 0 : g;
            img->data[y][x].blue = (b > 255) ? 255 : (b < 0) ? 0 : b;
        }
    }
}

t_pixel bmp24_convolution(t_bmp24 *img, int x, int y, float **kernel, int kernelSize) {
    t_pixel result = {0, 0, 0};
    if (!img || !img->data || !kernel || kernelSize % 2 == 0) return result;

    int n = kernelSize / 2;
    float sum_r = 0.0, sum_g = 0.0, sum_b = 0.0;

    for (int ky = -n; ky <= n; ky++) {
        for (int kx = -n; kx <= n; kx++) {
            int px = x + kx;
            int py = y + ky;

            if (px < 0) px = 0;
            if (px >= img->width) px = img->width - 1;
            if (py < 0) py = 0;
            if (py >= img->height) py = img->height - 1;

            float weight = kernel[ky + n][kx + n];
            sum_r += img->data[py][px].red * weight;
            sum_g += img->data[py][px].green * weight;
            sum_b += img->data[py][px].blue * weight;
        }
    }

    result.red = (sum_r > 255) ? 255 : (sum_r < 0) ? 0 : (uint8_t)sum_r;
    result.green = (sum_g > 255) ? 255 : (sum_g < 0) ? 0 : (uint8_t)sum_g;
    result.blue = (sum_b > 255) ? 255 : (sum_b < 0) ? 0 : (uint8_t)sum_b;

    return result;
}

void bmp24_applyFilter(t_bmp24 *img, float **kernel, int kernelSize) {
    if (!img || !img->data || !kernel || kernelSize % 2 == 0) return;

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

float **createKernel(const float *values, int size) {
    float **kernel = (float **)malloc(size * sizeof(float *));
    if (!kernel) return NULL;

    for (int i = 0; i < size; i++) {
        kernel[i] = (float *)malloc(size * sizeof(float));
        if (!kernel[i]) {
            for (int j = 0; j < i; j++) free(kernel[j]);
            free(kernel);
            return NULL;
        }

        for (int j = 0; j < size; j++) {
            kernel[i][j] = values[i * size + j];
        }
    }

    return kernel;
}

void freeKernel(float **kernel, int size) {
    if (!kernel) return;

    for (int i = 0; i < size; i++) {
        free(kernel[i]);
    }
    free(kernel);
}

void bmp24_boxBlur(t_bmp24 *img) {
    const float boxBlurKernel[] = {
        1.0/9, 1.0/9, 1.0/9,
        1.0/9, 1.0/9, 1.0/9,
        1.0/9, 1.0/9, 1.0/9
    };

    float **kernel = createKernel(boxBlurKernel, 3);
    if (kernel) {
        bmp24_applyFilter(img, kernel, 3);
        freeKernel(kernel, 3);
    }
}

void bmp24_gaussianBlur(t_bmp24 *img) {
    const float gaussianBlurKernel[] = {
        1.0/16, 2.0/16, 1.0/16,
        2.0/16, 4.0/16, 2.0/16,
        1.0/16, 2.0/16, 1.0/16
    };

    float **kernel = createKernel(gaussianBlurKernel, 3);
    if (kernel) {
        bmp24_applyFilter(img, kernel, 3);
        freeKernel(kernel, 3);
    }
}

void bmp24_outline(t_bmp24 *img) {
    const float outlineKernel[] = {
        -1, -1, -1,
        -1,  8, -1,
        -1, -1, -1
    };

    float **kernel = createKernel(outlineKernel, 3);
    if (kernel) {
        bmp24_applyFilter(img, kernel, 3);
        freeKernel(kernel, 3);
    }
}

void bmp24_emboss(t_bmp24 *img) {
    const float embossKernel[] = {
        -2, -1,  0,
        -1,  1,  1,
         0,  1,  2
    };

    float **kernel = createKernel(embossKernel, 3);
    if (kernel) {
        bmp24_applyFilter(img, kernel, 3);
        freeKernel(kernel, 3);
    }
}

void bmp24_sharpen(t_bmp24 *img) {
    const float sharpenKernel[] = {
         0, -1,  0,
        -1,  5, -1,
         0, -1,  0
    };

    float **kernel = createKernel(sharpenKernel, 3);
    if (kernel) {
        bmp24_applyFilter(img, kernel, 3);
        freeKernel(kernel, 3);
    }
}

typedef struct {
    float y; 
    float u; 
    float v; 
} t_yuv;

t_yuv rgb_to_yuv(t_pixel rgb) {
    t_yuv yuv;
    yuv.y = 0.299 * rgb.red + 0.587 * rgb.green + 0.114 * rgb.blue;
    yuv.u = -0.14713 * rgb.red - 0.28886 * rgb.green + 0.436 * rgb.blue;
    yuv.v = 0.615 * rgb.red - 0.51499 * rgb.green - 0.10001 * rgb.blue;
    return yuv;
}

t_pixel yuv_to_rgb(t_yuv yuv) {
    t_pixel rgb;
    rgb.red = (uint8_t)fminf(255, fmaxf(0, yuv.y + 1.13983 * yuv.v));
    rgb.green = (uint8_t)fminf(255, fmaxf(0, yuv.y - 0.39465 * yuv.u - 0.58060 * yuv.v));
    rgb.blue = (uint8_t)fminf(255, fmaxf(0, yuv.y + 2.03211 * yuv.u));
    return rgb;
}

void bmp24_equalize(t_bmp24 *img) {
    if (!img || !img->data) return;

    unsigned int *hist = (unsigned int *)calloc(256, sizeof(unsigned int));
    if (!hist) return;

    t_yuv **yuvImage = (t_yuv **)malloc(img->height * sizeof(t_yuv *));
    if (!yuvImage) {
        free(hist);
        return;
    }

    for (int y = 0; y < img->height; y++) {
        yuvImage[y] = (t_yuv *)malloc(img->width * sizeof(t_yuv));
        if (!yuvImage[y]) {
            for (int i = 0; i < y; i++) free(yuvImage[i]);
            free(yuvImage);
            free(hist);
            return;
        }

        for (int x = 0; x < img->width; x++) {
            yuvImage[y][x] = rgb_to_yuv(img->data[y][x]);
            int y_val = (int)yuvImage[y][x].y;
            if (y_val < 0) y_val = 0;
            if (y_val > 255) y_val = 255;
            hist[y_val]++;
        }
    }

    unsigned int *cdf = (unsigned int *)malloc(256 * sizeof(unsigned int));
    if (!cdf) {
        for (int y = 0; y < img->height; y++) free(yuvImage[y]);
        free(yuvImage);
        free(hist);
        return;
    }

    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i-1] + hist[i];
    }

    unsigned int cdf_min = 0;
    for (int i = 0; i < 256; i++) {
        if (cdf[i] > 0) {
            cdf_min = cdf[i];
            break;
        }
    }

    int total_pixels = img->width * img->height;
    unsigned char *equalized_y = (unsigned char *)malloc(256 * sizeof(unsigned char));
    if (!equalized_y) {
        for (int y = 0; y < img->height; y++) free(yuvImage[y]);
        free(yuvImage);
        free(hist);
        free(cdf);
        return;
    }

    for (int i = 0; i < 256; i++) {
        if (hist[i] > 0) {
            equalized_y[i] = (unsigned char)roundf(((float)(cdf[i] - cdf_min) /
                                                  (total_pixels - cdf_min)) * 255);
        } else {
            equalized_y[i] = 0;
        }
    }

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int y_val = (int)yuvImage[y][x].y;
            if (y_val < 0) y_val = 0;
            if (y_val > 255) y_val = 255;

            yuvImage[y][x].y = equalized_y[y_val];

            img->data[y][x] = yuv_to_rgb(yuvImage[y][x]);
        }
    }

    for (int y = 0; y < img->height; y++) free(yuvImage[y]);
    free(yuvImage);
    free(hist);
    free(cdf);
    free(equalized_y);
}
