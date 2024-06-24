#pragma once

#include <string>
#include <optional>
#include "audio/choc_MIDI.h"

using namespace std;
using choc::midi::ShortMessage;

namespace tstudio
{
    struct MidiMsg : public ShortMessage
    {
        bool isHandled = false;
        optional<string> device;
        optional<int> index;
        MidiMsg(){};

        MidiMsg(const void *sourceData, size_t numBytes, optional<string> device = nullopt, optional<int> index = nullopt);
        MidiMsg(uint8_t byte0, uint8_t byte1, uint8_t byte2, optional<string> device = nullopt, optional<int> index = nullopt)
            : ShortMessage(byte0, byte1, byte2), device(device), index(index){};
        ~MidiMsg();
    };
}
