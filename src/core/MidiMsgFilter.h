#pragma once

#include <string>
#include "core/MidiMsg.h"

using namespace std;

namespace tstudio
{
    class MidiMsgFilter
    {
    private:
        /* data */
    public:
        string device;
        uint8_t channel;
        MidiMsgFilter();
        MidiMsgFilter(string device, uint8_t channel);
        ~MidiMsgFilter();
        bool match(MidiMsg &event);
    };
}

