#pragma once


#include <algorithm>
#ifndef PUSH2GUI_H
#define PUSH2GUI_H

#include "cairomm/cairomm.h"
#include "core/Daw.h"
#include "hardware/push2/PixelBitmap.h"
#include "hardware/push2/Push2Display.h"
#include <atomic>
#include <cairomm/context.h>
#include <cairomm/refptr.h>
#include <cairomm/surface.h>
#include <iostream>
#include <thread>
#include <string>
#include <algorithm>

using namespace Cairo;

class Push2Gui
{
public:
    std::atomic<bool> running{true};
    Push2Display _display;
    RefPtr<ImageSurface> _surface;
    RefPtr<Context> _ctx;
    Push2DisplayBitmap _g;
    tstudio::Daw &daw;
    
    Push2Gui(tstudio::Daw &daw) : daw(daw){
        _surface = ImageSurface::create(ImageSurface::Format::RGB16_565, kWidth, kHeight);
        _ctx = Context::create(_surface);
        _display.Init();
        set_frames_per_second(_frames_per_second);
    };
    ~Push2Gui(){

    };
    const int16_t kWidth = 960;
    const int16_t kHeight = 160;

    void convert_and_fill_surface(){

      Push2DisplayBitmap::pixel_t *data = _g.PixelData();

      // A call to cairo_surface_flush() is required before accessing the pixel data
      _surface->flush();
      const uint8_t *image_data = _surface->get_data();

      // xor the pixel data
      static const uint16_t xOrMasks[2] = {0xf3e7, 0xffe7};

      // Get the image stride. Necessary for proper image data access
      // The stride is the distance in bytes from the beginning of one row of
      // the image data to the beginning of the next row.
      int image_stride = _surface->get_stride();

      // Convert the image data to push2 format
      for (int y = 0; y < kHeight; y++) {
        for (int x = 0; x < kWidth * 2; x += 2) {

          uint16_t rgb565 =
              image_data[y * image_stride + x] |
              (image_data[y * image_stride + x + 1] << 8); // Read 16-bit RGB565
          uint16_t r = (rgb565 >> 11) & 0x1F;       // Extract Red (5 bits)
          uint16_t g = (rgb565 >> 5) & 0x3F;        // Extract Green (6 bits)
          uint16_t b = rgb565 & 0x1F;               // Extract Blue (5 bits)

          uint16_t bgr565 = (b << 11) | (g << 5) | r; // Swap Red and Blue
          
          // Fill the pixel data with the converted image data
          *data++ = bgr565 ^ xOrMasks[(x / 2) % 2];
        }
      }
    };
    void render(tstudio::Daw &daw){
        _ctx->set_source_rgba(0.0, 0.0, 0.0, 1.0);
        _ctx->rectangle(0, 0, kWidth, kHeight);
        _ctx->fill();

        for (int i = 0; i < daw.session->tracks.size(); i++)
        {
            auto track = daw.session->tracks[i];
            // std::cout << "Track level" << to_string( track->meterNode->rmsDb()[0]) << "\n";
            float rms_db = track->meterNode->rmsDb()[0];
            float rms_db_scaled = (rms_db + 72) / 72;
            auto start_x = 0;
            float track_width = 120;
            float track_header_height = 120;
            auto end_x = start_x + track_width * i;
            _ctx->set_source_rgb(1.0, 0.0, 0.0);
            _ctx->rectangle(start_x, kHeight - rms_db_scaled * track_header_height, track_width-1, kHeight);
            _ctx->fill();
        }
    };
    void run (){
        while (running)
        {
        
        // Start the frame timer
        auto start_time = std::chrono::steady_clock::now();

        // Draw the GUI in a render function
        render(daw);
       

        // Convert the image data to push2 format
        convert_and_fill_surface();

        // Flip the display to the buffer
        _display.Flip(_g);

        // Sleep for 33ms
        auto frame_time = std::chrono::steady_clock::now() - start_time;
        std::this_thread::sleep_for(
            std::chrono::milliseconds(_millis_per_frame) - frame_time);
        }
    };
    void set_running(bool run){
        running = run;
    };


    void set_frames_per_second(int fps){

        _frames_per_second = std::clamp(fps, 1, 60);
        _millis_per_frame = (int)round(1000.0f / (float)_frames_per_second);
    };

protected:
 int _frames_per_second = 30;
 int _millis_per_frame;
    
};  

#endif // !PUSH2GUI_H