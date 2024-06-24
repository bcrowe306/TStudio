#include "string"
#include "memory"
#include "rtmidi/RtMidi.h"

#pragma once

namespace tstudio
{
    class MidiInPort
    {

    public:
        std::string name;
        int index;
        std::shared_ptr<RtMidiIn> port;
        bool enabled;
        MidiInPort(std::string name, int index, std::shared_ptr<RtMidiIn> port, bool enabled = false)
        {
            this->name = name;
            this->index = index;
            this->port = port;
            this->enabled = enabled;
        };
    };
    class MidiOutPort
    {

    public:
        std::string name;
        int index;
        std::shared_ptr<RtMidiOut> port;

        bool enabled;
        MidiOutPort(std::string name, int index, std::shared_ptr<RtMidiOut> port, bool enabled = false)
        {
            this->name = name;
            this->index = index;
            this->port = port;
            this->enabled = enabled;
        };
        void sendMessage(unsigned char status, unsigned char data1, unsigned char data2)
        {
            std::vector<unsigned char> midiMessage;
            midiMessage.push_back(status);
            midiMessage.push_back(data1);
            midiMessage.push_back(data2);
            port->sendMessage(&midiMessage);
        }
    };
}

