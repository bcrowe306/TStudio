#ifndef TRACKNODE_H
#define TRACKNODE_H

#include "LabSound//LabSound.h"
#include "LabSound/core/AnalyserNode.h"
#include "LabSound/core/AudioBasicInspectorNode.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/core/AudioNode.h"
#include "LabSound/core/AudioNodeOutput.h"
#include "LabSound/core/GainNode.h"
#include "LabSound/core/StereoPannerNode.h"
#include "core/InstrumentDevice.h"
#include "core/MidiEventRegistry.h"
#include "core/MidiMsg.h"
#include "core/MidiMsgFilter.h"
#include "core/InstrumentDevice.h"
#include "uuid.h"
#include <memory>
#include <string>



using namespace lab;
using namespace placeholders;
using std::make_shared;
using std::shared_ptr;
using std::string;

namespace tstudio {
    class TrackNode {
    public:
        string name;
        uuids::uuid id;
        shared_ptr<GainNode> volume;
        shared_ptr<InstrumentDevice> instrument;
        shared_ptr<StereoPannerNode> pan;
        shared_ptr<AnalyserNode> input;
        shared_ptr<AnalyserNode> output;
        shared_ptr<AudioContext> context;

        tstudio::MidiEventRegistry &midiEventRegistry;
        tstudio::MidiMsgFilter midiMsgFilter;
        TrackNode(shared_ptr<AudioContext> context) : context(context), midiEventRegistry(MidiEventRegistry::getInstance()) {

            // Setup midi stuff
            midiMsgFilter = MidiMsgFilter{"all", 0};
            midiEventRegistry.subscribe(
                midiMsgFilter, std::bind(&TrackNode::onMidiMsg, this, _1));
            volume = make_shared<GainNode>(*this->context);
            pan = make_shared<StereoPannerNode>(*this->context);
            input = make_shared<AnalyserNode>(*context);
            output = make_shared<AnalyserNode>(*context);
            this->context->connect(pan, input);
            this->context->connect(volume, pan);
            this->context->connect(output, volume);
        }
        void set_midi_device(std::string device){
            midiMsgFilter.device = device;
        }
        void set_midi_channel(uint8_t channel){
            midiMsgFilter.channel = channel;
        }
        void set_instrument(shared_ptr<InstrumentDevice> instrument){
            this->instrument = instrument;
            this->context->connect(output, this->instrument->output);
        }
        void onMidiMsg(MidiMsg &msg){
            if(instrument.get() != nullptr){
                instrument->onMidiMsg( msg);
            }
        }

    };
} // namespace tstudio

#endif