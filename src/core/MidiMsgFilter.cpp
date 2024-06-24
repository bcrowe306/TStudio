#include "core/MidiMsgFilter.h"

namespace tstudio
{
    MidiMsgFilter::MidiMsgFilter()
        : device("all"), channel(0)
    {
    }
    MidiMsgFilter::MidiMsgFilter(string device, unsigned char channel)
        : device(device), channel(channel)
    {
    }

    MidiMsgFilter::~MidiMsgFilter()
    {
    }

    bool MidiMsgFilter::match(MidiMsg &event)
    {
        bool deviceMatch = false;
        bool channelMatch = false;

        if (event.device.has_value())
        {
            if (device == "all" || device == event.device.value())
            {
                deviceMatch = true;
            }
        }
        else
        {
            deviceMatch = true;
        }

        if (channel == 0 || (uint8_t)channel == event.getChannel1to16())
        {
            channelMatch = true;
        }

        return deviceMatch && channelMatch;
    };
}
