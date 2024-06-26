#include <iostream>
#include <cairo.h>
#include "hardware/PixelBitmap.h"
#include "hardware/Push2Display.h"

#pragma once

struct RGB
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

bool isLittleEndian()
{
    uint16_t number = 1;
    return *(reinterpret_cast<uint8_t *>(&number)) == 1;
}

// Function to get pixel data from a pointer in CAIRO_FORMAT_RGB24
RGB getPixel(const uint8_t *data, int width, int height, int x, int y, int stride)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        std::cout << "Error: Pixel coordinates out of bounds" << std::endl;
        return {0, 0, 0}; // Return black or handle as needed
    }

    // Calculate the position in the data array
    const uint32_t *pixelAddr = reinterpret_cast<const uint32_t *>(data + y * stride + x * 4); // 4 bytes per pixel for CAIRO_FORMAT_RGB24
    uint32_t pixelValue = *pixelAddr;

    RGB pixel;
    if (isLittleEndian())
    {
        // Little-endian system
        pixel.r = (pixelValue >> 16) & 0xFF; // Extract bits 16-23
        pixel.g = (pixelValue >> 8) & 0xFF;  // Extract bits 8-15
        pixel.b = pixelValue & 0xFF;         // Extract bits 0-7
    }
    else
    {
        // Big-endian system
        pixel.r = pixelValue & 0xFF;         // Extract bits 0-7
        pixel.g = (pixelValue >> 8) & 0xFF;  // Extract bits 8-15
        pixel.b = (pixelValue >> 16) & 0xFF; // Extract bits 16-23
    }

    return pixel;
}

class Push2
{
private:
    /* data */
public:
    int height = 160;
    int width = 960;
    Push2Display *display;
    Push2(/* args */)
    {
        display = new Push2Display();
        display->Init();
    };
    ~Push2(){
        delete display;
    };
    void draw(Widget &root){
        cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
        cairo_t *cr = cairo_create(surface);
        root.draw(cr);
        cairo_surface_flush(surface);
        
        // Get pointer for beginning of image data in memory
        unsigned char *imgae_data_ptr = cairo_image_surface_get_data(surface);
        // Get stride in bytes for cairo surface
        int stride = cairo_image_surface_get_stride(surface);

        // Initialize the pixelBitmap object and memory
        Push2DisplayBitmap g;
        Push2DisplayBitmap::pixel_t *data = g.PixelData();

        // Create the xor mask needed for the push2
        static const uint16_t xOrMasks[2] = {0xf3e7, 0xffe7};

        // Get individual pixel data
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                RGB rgb = getPixel(imgae_data_ptr, width, height, x, y, stride);
                const auto pixel = Push2DisplayBitmap::SPixelFromRGB(rgb.r, rgb.g, rgb.b);
                *data++ = pixel ^ xOrMasks[x % 2];
            }
            data += (g.GetWidth() - width);
        }

        display->Flip(g);
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
    };
};
