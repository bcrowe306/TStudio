#pragma once

#include "Push2Communicator.h"
#include "PixelBitmap.h"
#include <memory>
#include <atomic>

    //=====================================================================

    class Push2Display
    {
    public:
        using pixel_t = Push2DisplayBitmap::pixel_t;

        Push2Display()
        {
            pixel_t *pData = dataSource_;
            for (uint8_t line = 0; line < kDataSourceHeight; line++)
            {
                memset(pData, 0, kDataSourceWidth * sizeof(pixel_t));
                pData += kDataSourceWidth;
            }
        }

        void Init()
        {
            return communicator_.Init(dataSource_);
        }

        // Transfers the bitmap into the output buffer sent to
        // the push display. The push display buffer has a larger stride
        // as the given bitmap

        void Flip(const Push2DisplayBitmap &g)
        {
            const pixel_t *src = g.PixelData();
            pixel_t *dst = dataSource_;

            const int graphicsWidth = g.GetWidth();
            assert(g.GetHeight() == kDataSourceHeight);
            for (uint8_t line = 0; line < kDataSourceHeight; line++)
            {
                memcpy(dst, src, graphicsWidth * sizeof(pixel_t));
                src += graphicsWidth;
                dst += kDataSourceWidth;
            }
        }

    private:
        static const int kDataSourceWidth = 1024;
        static const int kDataSourceHeight = 160;

        pixel_t dataSource_[kDataSourceWidth * kDataSourceHeight];
        Push2Communicator communicator_;
    };