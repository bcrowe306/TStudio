#include <cstdint>
#include "thread"
#include <assert.h>
#include <iostream>
#include <atomic>

#ifdef _WIN32
// see following link for a discussion of the
// warning suppression:
// http://sourceforge.net/mailarchive/forum.php?
// thread_name=50F6011C.2020000%40akeo.ie&forum_name=libusbx-devel

// Disable: warning C4200: nonstandard extension used:
// zero-sized array in struct/union
#pragma warning(disable : 4200)
#endif

#pragma once
#include "libusb.h"
#include "PixelBitmap.h"



int SFindPushDisplayDeviceHandle(libusb_device_handle **pHandle)
{
    int errorCode;

    // Initialises the library
    if ((errorCode = libusb_init(NULL)) < 0)
    {
        printf("Failed to initialize usblib\n");
        return errorCode;
    }

    libusb_set_debug(NULL, LIBUSB_LOG_LEVEL_ERROR);

    // Get a list of connected devices
    libusb_device **devices;
    ssize_t count;
    count = libusb_get_device_list(NULL, &devices);
    if (count < 0)
    {
        printf("could not get usb device list\n");
        return errorCode;
    }

    // Look for the one matching push2's decriptors
    libusb_device *device;
    libusb_device_handle *device_handle = NULL;

    char ErrorMsg[256];

    // set message in case we get to the end of the list w/o finding a device
    snprintf(ErrorMsg, 256,  "display device not found\n");

    for (int i = 0; (device = devices[i]) != NULL; i++)
    {
        struct libusb_device_descriptor descriptor;
        if ((errorCode = libusb_get_device_descriptor(device, &descriptor)) < 0)
        {
            snprintf(ErrorMsg, 256, "could not get usb device descriptor, error: %d", errorCode);
            continue;
        }

        const uint16_t kAbletonVendorID = 0x2982;
        const uint16_t kPush2ProductID = 0x1967;

        if (descriptor.bDeviceClass == LIBUSB_CLASS_PER_INTERFACE && descriptor.idVendor == kAbletonVendorID && descriptor.idProduct == kPush2ProductID)
        {
            std::cout << "Found Ableton Push 2 Device\n";
            if ((errorCode = libusb_open(device, &device_handle)) < 0)
            {
                snprintf(ErrorMsg, 256, "could not open device, error: %d", errorCode);
            }
            else if ((errorCode = libusb_claim_interface(device_handle, 0)) < 0)
            {
                snprintf(ErrorMsg, 256, "could not claim device with interface 0, error: %d", errorCode);
                libusb_close(device_handle);
                device_handle = NULL;
            }
            else
            {
                std::cout << "Successfully opened Push 2 Device\n";
                break; // successfully opened
            }
        }
    }

    *pHandle = device_handle;
    libusb_free_device_list(devices, 1);

    return errorCode;
}

//------------------------------------------------------------------------------
class Push2Communicator
{
public:
    using pixel_t = Push2DisplayBitmap::pixel_t;

    // The display frame size is 960*160*2=300k, but we use 64 extra filler
    // pixels per line so that we get exactly 2048 bytes per line. The purpose
    // is that the device receives exactly 4 buffers of 512 bytes each per line,
    // so that the line boundary (which is where we save to SDRAM) does not fall
    // into the middle of a received buffer. Therefore we actually send
    // 1024*160*2=320k bytes per frame.

    static const int kLineSize = 2048; // total line size
    static const int kLineCountPerSendBuffer = 8;

    // The data sent to the display is sliced into chunks of kLineCountPerSendBuffer
    // and we use kSendBufferCount buffers to communicate so we can prepare the next
    // request without having to wait for the current one to be finished
    // The sent buffer size (kSendBufferSize) must a multiple of these 2k per line,
    // and is selected for optimal performance.

    static const int kSendBufferCount = 3;
    static const int kSendBufferSize = kLineCountPerSendBuffer * kLineSize; // buffer length in bytes

    Push2Communicator() : handle_(NULL){};
    ~Push2Communicator()
    {
        // shutdown the polling thread
        terminate_ = true;
        if (pollThread_.joinable())
        {
            pollThread_.join();
        }
    }

    /*!
     *  Inialises the communicator. This will look for the usb descriptor matching
     *  the display, allocate transfer buffers and start sending data.
     *
     *  \param dataSource: The buffer holding the data to be sent to the display.
     *  \return the result of the initialisation
     */

    void Init(const pixel_t *dataSource)
    {
        // Capture the data source
        dataSource_ = dataSource;

        // Initialise the handle
        SFindPushDisplayDeviceHandle(&handle_);
        assert(handle_ != NULL);

        // Initialise the transfer
        startSending();
        // We initiate a thread so we can recieve events from libusb
        terminate_ = false;
        pollThread_ = std::thread(&Push2Communicator::PollUsbForEvents, this);
    };

    /*!
     *  Callback for when a transfer is finished and the next one needs to be
     *  initiated
     */

    void OnTransferFinished(libusb_transfer *transfer)
    {
        if (transfer->status != LIBUSB_TRANSFER_COMPLETED)
        {
            assert(0);
            switch (transfer->status)
            {
            case LIBUSB_TRANSFER_ERROR:
                printf("transfer failed\n");
                break;
            case LIBUSB_TRANSFER_TIMED_OUT:
                printf("transfer timed out\n");
                break;
            case LIBUSB_TRANSFER_CANCELLED:
                printf("transfer was cancelled\n");
                break;
            case LIBUSB_TRANSFER_STALL:
                printf("endpoint stalled/control request not supported\n");
                break;
            case LIBUSB_TRANSFER_NO_DEVICE:
                printf("device was disconnected\n");
                break;
            case LIBUSB_TRANSFER_OVERFLOW:
                printf("device sent more data than requested\n");
                break;
            default:
                printf("snd transfer failed with status: %d\n", transfer->status);
                break;
            }
        }
        else if (transfer->length != transfer->actual_length)
        {
            assert(0);
            printf("only transferred %d of %d bytes\n", transfer->actual_length, transfer->length);
        }
        else if (transfer == frameHeaderTransfer_)
        {
            onFrameCompleted();
        }
        else
        {
            sendNextSlice(transfer);
        }
    };

    /*!
     *  Continuously poll events from libusb, possibly treating any error reported
     */

    void PollUsbForEvents()
    {
        static struct timeval timeout_500ms = {0, 500000};
        int terminate_main_loop = 0;

        while (!terminate_main_loop && !terminate_.load())
        {
            if (libusb_handle_events_timeout_completed(NULL, &timeout_500ms, &terminate_main_loop) < 0)
            {
                assert(0);
            }
        }
    };

private:
    /*!
     *  Initiate the send process
     */

    void startSending()
    {
        currentLine_ = 0;

        // Allocates a transfer struct for the frame header

        static unsigned char frameHeader[16] =
            {
                0xFF, 0xCC, 0xAA, 0x88,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00};

        frameHeaderTransfer_ =
            SAllocateAndPrepareTransferChunk(frameHeader, sizeof(frameHeader));

        for (int i = 0; i < kSendBufferCount; i++)
        {
            unsigned char *buffer = (sendBuffers_ + i * kSendBufferSize);

            // Allocates a transfer struct for the send buffers

            libusb_transfer *transfer =
                SAllocateAndPrepareTransferChunk(buffer, kSendBufferSize);

            // Start a request for this buffer
            sendNextSlice(transfer);
        }
    };

    /*!
     *  Send the next slice of data using the provided transfer struct
     */
    libusb_transfer *SAllocateAndPrepareTransferChunk(unsigned char *buffer, int bufferSize)
    {
        // Allocate a transfer structure
        libusb_transfer *transfer = libusb_alloc_transfer(0);
        if (!transfer)
        {
            return nullptr;
        }

        // Sets the transfer characteristic
        const unsigned char kPush2BulkEPOut = 0x01;

        libusb_fill_bulk_transfer(transfer, handle_, kPush2BulkEPOut,
                                  buffer, bufferSize,
                                  SOnTransferFinished, this, 1000);
        return transfer;
    };
    
    void sendNextSlice(libusb_transfer *transfer)
    {

        // Start of a new frame, so send header first
        if (currentLine_ == 0)
        {
            if (libusb_submit_transfer(frameHeaderTransfer_) < 0)
            {
                printf("could not submit frame header transfer\n");
            }
        }

        // Copy the next slice of the source data (represented by currentLine_)
        // to the transfer buffer

        unsigned char *dst = transfer->buffer;

        const char *src = (const char *)dataSource_ + kLineSize * currentLine_;
        unsigned char *end = dst + kSendBufferSize;

        while (dst < end)
        {
            *dst++ = *src++;
        }

        // Send it
        if (libusb_submit_transfer(transfer) < 0)
        {
            printf("could not submit display data transfer,\n");
        }

        // Update slice position
        currentLine_ += kLineCountPerSendBuffer;

        if (currentLine_ >= 160)
        {
            currentLine_ = 0;
        }
    };
    static void SOnTransferFinished(libusb_transfer *transfer)
    {
        static_cast<Push2Communicator *>(transfer->user_data)->OnTransferFinished(transfer);
    };

    /*!
     *  Callback for when a full frame has been sent
     *  Note that there's no real need of doing double buffering since the
     *  display deals nicely with it already
     */

    void onFrameCompleted() {
        // Insert code here if you want anything to happen after each frame
    };

    const pixel_t *dataSource_;
    libusb_device_handle *handle_;
    libusb_transfer *frameHeaderTransfer_;
    std::thread pollThread_;
    uint8_t currentLine_;
    std::atomic<bool> terminate_;
    unsigned char sendBuffers_[kSendBufferCount * kSendBufferSize];
};

