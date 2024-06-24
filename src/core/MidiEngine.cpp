#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "core/MidiEngine.h"
#include "core/MidiHandler.h"

namespace tstudio {
    MidiEngine::MidiEngine(bool auto_enable)
    {
        this->midiEventRegistry = &MidiEventRegistry::getInstance();
        this->auto_enable = auto_enable;
        this->midiIn = std::make_shared<RtMidiIn>(RtMidiIn());
        this->midiOut = std::make_shared<RtMidiOut>(RtMidiOut());

        this->initialize();
    }

    void MidiEngine::refreshDevices()
    {
        for (unsigned int i = 0; i < midiIn->getPortCount(); i++)
        {
            auto device = midiIn->getPortName(i);
            inputDevices[device] = std::make_shared<MidiInPort>(device, (int)i, std::make_shared<RtMidiIn>(), auto_enable);
            inputCallbackMap[device] = MidiCallbackData{this, &(*inputDevices[device])};
            inputDevices[device]->port->setCallback(&midiCallback, &(inputCallbackMap[device]));
            // inputDevices[device]->port->openPort(i);
        }
        for (unsigned int i = 0; i < midiOut->getPortCount(); i++)
        {
            auto device = midiOut->getPortName(i);
            outputDevices[device] = std::make_shared<MidiOutPort>(device, (int)i, std::make_shared<RtMidiOut>(), auto_enable);
            // outputDevices[device]->port->openPort(i);
        }
    };
    void MidiEngine::setMidiOutQueue(choc::fifo::SingleReaderSingleWriterFIFO<MidiMsg> &midiOutQueue)
    {
        this->midiOutQueue = &midiOutQueue;
    };
    void MidiEngine::midiCallback(double timeStamp, std::vector<unsigned char> *message, void *userData)
    {
        // Get the user data pointer.
        MidiCallbackData *callbackData = (MidiCallbackData *)userData;
        MidiMsg msg{(*message)[0], (*message)[1], (*message)[2], callbackData->port->name, callbackData->port->index};

        // // Send the message to the midi engine for processing through process chain
        std::any_cast<MidiEngine *>(callbackData->engine)->process(msg);
    };

    void MidiEngine::activate()
    {
        for (auto &[name, midiInPort] : inputDevices)
        {
            if (midiInPort->enabled && !midiInPort->port->isPortOpen())
            {
                midiInPort->port->openPort(midiInPort->index);
            }
        }
        for (auto &[name, midiOutPort] : outputDevices)
        {
            if (midiOutPort->enabled && !midiOutPort->port->isPortOpen())
            {
                midiOutPort->port->openPort(midiOutPort->index);
            }
        }
    }
    void MidiEngine::deactivate()
    {
        for (auto &[name, midiInPort] : inputDevices)
        {
            if (midiInPort->port->isPortOpen())
            {
                midiInPort->port->closePort();
            }
        }
        for (auto &[name, midiOutPort] : outputDevices)
        {
            if (midiOutPort->port->isPortOpen())
            {
                midiOutPort->port->closePort();
            }
        }
    }

    void MidiEngine::initialize()
    {
        this->refreshDevices();
    }

    void MidiEngine::setInputDeviceEnabled(const std::string &name, bool enabled)
    {
        if (inputDevices.find(name) != inputDevices.end())
        {
            inputDevices[name]->enabled = enabled;
        }
    };

    void MidiEngine::setOutputDeviceEnabled(const std::string &name, bool enabled)
    {
        if (outputDevices.find(name) != outputDevices.end())
        {
            outputDevices[name]->enabled = enabled;
        }
    };
    void MidiEngine::process(MidiMsg &event)
    {
        midiEventRegistry->notify(event.device.value(), event);
    };
}
