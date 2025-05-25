#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp8.h"
#include "bmp24.h"

// ---- Menus ----
void printMainMenu() {
    printf("\nMain Menu:\n");
    printf("1. Open an 8-bit grayscale image\n");
    printf("2. Open a 24-bit color image\n");
    printf("3. Save current image\n");
    printf("4. Apply a filter\n");
    printf("5. Display image info\n");
    printf("6. Quit\n");
    printf(">>> Your choice: ");
}

void printFilterMenu8() {
    printf("\n8-bit Filters:\n");
    printf("1. Negative\n");
    printf("2. Brightness\n");
    printf("3. Black and white (Threshold)\n");
    printf("4. Box Blur\n");
    printf("5. Gaussian blur\n");
    printf("6. Sharpness\n");
    printf("7. Outline\n");
    printf("8. Emboss\n");
    printf("9. Return to previous menu\n");
    printf(">>> Your choice: ");
}

void printFilterMenu24() {
    printf("\n24-bit Filters:\n");
    printf("1. Negative\n");
    printf("2. Grayscale\n");
    printf("3. Brightness\n");
    printf("4. Box Blur\n");
    printf("5. Gaussian Blur\n");
    printf("6. Outline\n");
    printf("7. Emboss\n");
    printf("8. Sharpen\n");
    printf("9. Histogram Equalization\n");
    printf("10. Return to previous menu\n");
    printf(">>> Your choice: ");
}

// ---- Main ----
int main() {
    t_bmp8 *img8 = NULL;
    t_bmp24 *img24 = NULL;
    int currentType = 0;
    int choice;
    char filename[256];

    // Define convolution kernels
    float* boxBlurKernel[3] = {
        (float[]){1/9.0f, 1/9.0f, 1/9.0f},
        (float[]){1/9.0f, 1/9.0f, 1/9.0f},
        (float[]){1/9.0f, 1/9.0f, 1/9.0f}
    };

    float* gaussianBlurKernel[3] = {
        (float[]){1/16.0f, 2/16.0f, 1/16.0f},
        (float[]){2/16.0f, 4/16.0f, 2/16.0f},
        (float[]){1/16.0f, 2/16.0f, 1/16.0f}
    };

    float* outlineKernel[3] = {
        (float[]){-1, -1, -1},
        (float[]){-1,  8, -1},
        (float[]){-1, -1, -1}
    };

    float* embossKernel[3] = {
        (float[]){-2, -1, 0},
        (float[]){-1,  1, 1},
        (float[]){ 0,  1, 2}
    };

    float* sharpenKernel[3] = {
        (float[]){ 0, -1,  0},
        (float[]){-1,  5, -1},
        (float[]){ 0, -1,  0}
    };

    while (1) {
        printMainMenu();
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                if (img8) bmp8_free(img8);
                if (img24) { bmp24_free(img24); img24 = NULL; }
                currentType = 8;
                printf("Enter 8-bit image path: ");
                fgets(filename, 256, stdin);
                filename[strcspn(filename, "\n")] = 0;
                img8 = bmp8_loadImage(filename);
                printf(img8 ? "Image loaded.\n" : "Failed to load image.\n");
                break;

            case 2:
                if (img24) bmp24_free(img24);
                if (img8) { bmp8_free(img8); img8 = NULL; }
                currentType = 24;
                printf("Enter 24-bit image path: ");
                fgets(filename, 256, stdin);
                filename[strcspn(filename, "\n")] = 0;
                img24 = bmp24_loadImage(filename);
                printf(img24 ? "Image loaded.\n" : "Failed to load image.\n");
                break;

            case 3:
                printf("Enter save path: ");
                fgets(filename, 256, stdin);
                filename[strcspn(filename, "\n")] = 0;
                if (currentType == 8 && img8) bmp8_saveImage(filename, img8);
                else if (currentType == 24 && img24) bmp24_saveImage(img24, filename);
                else printf("No image loaded.\n");
                break;

            case 4:
                if (currentType == 24 && img24) {
                    int fchoice, value;
                    printFilterMenu24();
                    scanf("%d", &fchoice);
                    getchar();
                    switch (fchoice) {
                        case 1: bmp24_negative(img24); break;
                        case 2: bmp24_grayscale(img24); break;
                        case 3: printf("Brightness (-255 to 255): "); scanf("%d", &value); getchar(); bmp24_brightness(img24, value); break;
                        case 4: bmp24_boxBlur(img24); break;
                        case 5: bmp24_gaussianBlur(img24); break;
                        case 6: bmp24_outline(img24); break;
                        case 7: bmp24_emboss(img24); break;
                        case 8: bmp24_sharpen(img24); break;
                        case 9: bmp24_equalize(img24); break;
                        default: break;
                    }
                    printf("Filter applied.\n");
                } else if (currentType == 8 && img8) {
                    while (1) {
                        printFilterMenu8();
                        scanf("%d", &choice);
                        getchar();
                        if (choice == 9) break;
                        switch (choice) {
                            case 1: bmp8_negative(img8); break;
                            case 2: printf("Brightness: "); int v; scanf("%d", &v); getchar(); bmp8_brightness(img8, v); break;
                            case 3: printf("Threshold (0-255): "); int t; scanf("%d", &t); getchar(); bmp8_threshold(img8, t); break;
                            case 4: bmp8_applyFilter(img8, boxBlurKernel, 3); break;
                            case 5: bmp8_applyFilter(img8, gaussianBlurKernel, 3); break;
                            case 6: bmp8_applyFilter(img8, sharpenKernel, 3); break;
                            case 7: bmp8_applyFilter(img8, outlineKernel, 3); break;
                            case 8: bmp8_applyFilter(img8, embossKernel, 3); break;
                            default: printf("Invalid choice.\n");
                        }
                        printf("Filter applied.\n");
                    }
                } else {
                    printf("No image loaded.\n");
                }
                break;

            case 5:
                if (currentType == 8 && img8) bmp8_printInfo(img8);
                else if (currentType == 24 && img24) {
                    printf("Image Info:\n");
                    printf("    Width: %d\n", img24->width);
                    printf("    Height: %d\n", img24->height);
                    printf("    Color Depth: %d\n", img24->colorDepth);
                } else printf("No image loaded.\n");
                break;

            case 6:
                if (img8) bmp8_free(img8);
                if (img24) bmp24_free(img24);
                return 0;

            default:
                printf("Invalid option.\n");
        }
    }
}
