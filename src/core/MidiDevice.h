#pragma once

#ifndef MIDIDEVICE_H
#define MIDIDEVICE_H

#include "memory"
#include "rtmidi/RtMidi.h"
#include "string"

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
        void set_track_enabeld(bool enabled)
        {
            this->_track = enabled;
        }
        bool is_track_enabled()
        {
            return this->_track;
        }

        void set_sync_enabled(bool enabled)
        {
            this->_sync = enabled;
        }
        bool is_sync_enabled()
        {
            return this->_sync;
        }
        
        void set_remote_enabled(bool enabled)
        {
            this->_remote = enabled;
        }
        bool is_remote_enabled()
        {
            return this->_remote;
        }
    protected:
        bool _track;
        bool _sync;
        bool _remote;

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
        void set_track_enabeld(bool enabled) { this->_track = enabled; }
        bool is_track_enabled() { return this->_track; }

        void set_sync_enabled(bool enabled) { this->_sync = enabled; }
        bool is_sync_enabled() { return this->_sync; }

        void set_remote_enabled(bool enabled) { this->_remote = enabled; }
        bool is_remote_enabled() { return this->_remote; }

      protected:
        bool _track;
        bool _sync;
        bool _remote;
    };
}


#endif // !MIDIDEVICE_H


