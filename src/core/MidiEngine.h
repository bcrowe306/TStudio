#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <any>
#include "rtmidi/RtMidi.h"
#include "core/MidiHandler.h"
#include "core/MidiMsg.h"
#include "core/MidiDevice.h"
#include "containers/choc_SingleReaderSingleWriterFIFO.h"
#include "core/MidiEventRegistry.h"

#pragma once

namespace tstudio {
    
    struct MidiCallbackData
    {
        std::any engine;
        MidiInPort *port;
    };

    class MidiEngine : public MidiHandler
    {
    public:
        std::shared_ptr<RtMidiIn> midiIn;
        std::shared_ptr<RtMidiOut> midiOut;
        std::unordered_map<std::string, std::shared_ptr<MidiInPort>> inputDevices;
        std::unordered_map<std::string, std::shared_ptr<MidiOutPort>> outputDevices;
        std::unordered_map<std::string, MidiCallbackData> inputCallbackMap;
        choc::fifo::SingleReaderSingleWriterFIFO<MidiMsg> *midiOutQueue;
        MidiEventRegistry *midiEventRegistry;

        bool auto_enable;

        MidiEngine(bool auto_enable = false);

        void printOutputDevices();
        static void midiCallback(double timeStamp, std::vector<unsigned char> *message, void *userData);

        void initialize();

        
        void setMidiOutQueue(choc::fifo::SingleReaderSingleWriterFIFO<MidiMsg> &midiOutQueue);
        
        void setInputDeviceEnabled(const std::string &name, bool enabled);

        void setOutputDeviceEnabled(const std::string &name, bool enabled);

        void setInputDeviceSyncEnabled(const std::string &name, bool enabled);
        void setInputDeviceRemoteEnabled(const std::string &name, bool enabled);
        void setInputDeviceTrackEnabled(const std::string &name, bool enabled);

        void setOutputDeviceSyncEnabled(const std::string &name, bool enabled);
        void setOutputDeviceRemoteEnabled(const std::string &name, bool enabled);
        void setOutputDeviceTrackEnabled(const std::string &name, bool enabled);

        void onMidiOut(const MidiMsg &event);

        void refreshDevices();

        void activate();

        void deactivate();

        void process(MidiMsg &event) override;

        void onMidiClockOut();

    private:
        // MidiEventRegistry midiEventRegistry;
        // std::vector<std::tuple<std::string, std::string, ControlSurface *>> controlSurfaces;
    };
}


