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
#include "core/MidiClip.h"
#include "core/MidiEventRegistry.h"
#include "core/MidiMsg.h"
#include "core/MidiMsgFilter.h"
#include "core/MidiNode.h"
#include "core/InstrumentDevice.h"
#include "library/Parameter.h"
#include "uuid.h"
#include <memory>
#include <string>
#include <vector>



using namespace lab;
using namespace placeholders;
using std::make_shared;
using std::vector;
using std::shared_ptr;
using std::string;

namespace tstudio {
    class TrackNode : public MidiNode {
    public:
        uuids::uuid id;
        FloatParam volume = FloatParam("volume", "Voluem", 1.f, "Slider");
        BoolParam mute = BoolParam("mute", "Mute", false, "Toggle");
        BoolParam solo = BoolParam("solo", "Solo", false, "Toggle");
        BoolParam arm = BoolParam("arm", "arm", false, "Toggle");
        FloatParam pan = FloatParam("pan", "Pan", .5f, "Slider");
        shared_ptr<GainNode> volumeNode;
        shared_ptr<InstrumentDevice> instrument;
        shared_ptr<StereoPannerNode> panNode;
        shared_ptr<AnalyserNode> input;
        shared_ptr<AnalyserNode> output;
        shared_ptr<AudioContext> context;
        vector<shared_ptr<MidiClip>> clips;
        tstudio::MidiEventRegistry &midiEventRegistry;
        tstudio::MidiMsgFilter midiMsgFilter;
        TrackNode(shared_ptr<AudioContext>);
        void add_clip(shared_ptr<MidiClip>);
        void remove_clip(shared_ptr<MidiClip>);
        void set_midi_device(std::string);
        void set_midi_channel(uint8_t);
        void set_instrument(shared_ptr<InstrumentDevice>);
        void onMidiMsg(MidiMsg &msg);
        void onMidiClipEvents(MidiMsg &);
    };
} // namespace tstudio

#endif