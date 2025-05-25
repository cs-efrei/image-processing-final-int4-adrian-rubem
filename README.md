P1 - TI202: Image Processing in C
Description

This project consists of creating a BMP image processing program in C. It is divided into three main parts:

    8-bit grayscale image processing

    24-bit color image processing

    Histogram equalization to improve contrast

Project Team

    Adrian Rubem

    Saïf Bousaada

Source Files

    bmp8.c / bmp8.h 

    bmp24.c / bmp24.h 

    main.c 

Data Structures

    t_bmp8: represents a grayscale image (8-bit), with header, color table, and pixel data

    t_bmp24: represents a 24-bit color image, with header, pixel matrix, and image metadata

    t_pixel: represents a color pixel (R, G, B values)

Implemented Features
Part 1: 8-bit Grayscale Images

    Load and save grayscale BMP files

    Display image information (width, height, depth, size)

    Apply filters: negative, brightness, threshold (black & white), convolution (blur, sharpen, etc.)

Part 2: 24-bit Color Images

    Load and save 24-bit BMP files

    Apply filters: negative, convert to grayscale, adjust brightness

    Convolution filters: box blur, Gaussian blur, sharpen, outline, emboss

Part 3: Histogram Equalization

    Compute grayscale histogram

    Compute cumulative normalized histogram (CDF)

    Equalize the image to enhance contrast

Not Implemented Features

    Histogram equalization for color images (YUV conversion)

Known Issues

    Some color images may not display correctly if not aligned properly (padding issue)

    No graphical interface (only CLI)

Task Distribution

    Adrian: File reading/writing, grayscale filters

    Saïf: Color filters, user interface, histogram equalization
