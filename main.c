#include <stdio.h>
#include <stdlib.h>
#include "bmp8.h"
#include <string.h>

void printMenu() {
    printf("\nPlease choose an option:\n");
    printf("1. Open an image\n");
    printf("2. Save an image\n");
    printf("3. Apply a filter\n");
    printf("4. Display image information\n");
    printf("5. Quit\n");
    printf(">>> Your choice: ");
}

void printFilterMenu() {
    printf("\nPlease choose a filter:\n");
    printf("1. Negative\n");
    printf("2. Brightness\n");
    printf("3. Black and white (Threshold)\n");
    printf("4. Box Blur\n");
    printf("5. Gaussian blur\n");
    printf("6. Sharpness\n");
    printf("7. Outline\n");
    printf("8. Emboss\n");
    printf("9. Return to the previous menu\n");
    printf(">>> Your choice: ");
}

int main() {
    t_bmp8* image = NULL;
    int choice;
    char filename[256];


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
        (float[]){-1, 8, -1},
        (float[]){-1, -1, -1}
    };

    float* embossKernel[3] = {
        (float[]){-2, -1, 0},
        (float[]){-1, 1, 1},
        (float[]){0, 1, 2}
    };

    float* sharpenKernel[3] = {
        (float[]){0, -1, 0},
        (float[]){-1, 5, -1},
        (float[]){0, -1, 0}
    };

    while (1) {
        printMenu();
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                if (image) {
                    bmp8_free(image);
                    image = NULL;
                }
                printf("File path: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = '\0'; // Remove newline
                image = bmp8_loadImage(filename);
                if (image) {
                    printf("Image loaded successfully!\n");
                }
                break;

            case 2:
                if (!image) {
                    printf("Error: No image loaded\n");
                    break;
                }
                printf("File path: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = '\0';
                bmp8_saveImage(filename, image);
                printf("Image saved successfully!\n");
                break;

            case 3:
                if (!image) {
                    printf("Error: No image loaded\n");
                    break;
                }

                while (1) {
                    printFilterMenu();
                    scanf("%d", &choice);
                    getchar();

                    if (choice == 9) break;

                    switch (choice) {
                        case 1:
                            bmp8_negative(image);
                            printf("Filter applied successfully!\n");
                            break;

                        case 2:
                            printf("Enter brightness value (-255 to 255): ");
                            int value;
                            scanf("%d", &value);
                            getchar();
                            bmp8_brightness(image, value);
                            printf("Filter applied successfully!\n");
                            break;

                        case 3:
                            printf("Enter threshold value (0-255): ");
                            int threshold;
                            scanf("%d", &threshold);
                            getchar();
                            bmp8_threshold(image, threshold);
                            printf("Filter applied successfully!\n");
                            break;

                        case 4:
                            bmp8_applyFilter(image, boxBlurKernel, 3);
                            printf("Filter applied successfully!\n");
                            break;

                        case 5:
                            bmp8_applyFilter(image, gaussianBlurKernel, 3);
                            printf("Filter applied successfully!\n");
                            break;

                        case 6:
                            bmp8_applyFilter(image, sharpenKernel, 3);
                            printf("Filter applied successfully!\n");
                            break;

                        case 7:
                            bmp8_applyFilter(image, outlineKernel, 3);
                            printf("Filter applied successfully!\n");
                            break;

                        case 8:
                            bmp8_applyFilter(image, embossKernel, 3);
                            printf("Filter applied successfully!\n");
                            break;

                        default:
                            printf("Invalid choice\n");
                    }
                }
                break;

            case 4:
                if (!image) {
                    printf("Error: No image loaded\n");
                    break;
                }
                bmp8_printInfo(image);
                break;

            case 5:
                if (image) {
                    bmp8_free(image);
                }
                return 0;

            default:
                printf("Invalid choice\n");
        }
    }
}