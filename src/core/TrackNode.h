#ifndef TRACKNODE_H
#define TRACKNODE_H

#include "LabSound//LabSound.h"
#include "core/Playhead.h"
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
#include <map>



using namespace lab;
using namespace placeholders;
using std::make_shared;
using std::vector;
using std::shared_ptr;
using std::unordered_map;
using std::string;

namespace tstudio {
    class TrackNode : public MidiNode
    {
    public:
        uuids::uuid id;
        int clipCount = 0;
        FloatParam volume = FloatParam("volume", "Voluem", 1.f, "Slider");
        BoolParam mute = BoolParam("mute", "Mute", false, "Toggle");
        BoolParam solo = BoolParam("solo", "Solo", false, "Toggle");
        BoolParam arm = BoolParam("arm", "arm", false, "Toggle");
        FloatParam pan = FloatParam("pan", "Pan", .5f, "Slider");
        StringParam color = StringParam("color", "Color", "", "Text");
        shared_ptr<GainNode> volumeNode;
        shared_ptr<InstrumentDevice> instrument;
        shared_ptr<StereoPannerNode> panNode;
        shared_ptr<AnalyserNode> input;
        shared_ptr<AnalyserNode> output;
        shared_ptr<AudioContext> context;
        shared_ptr<Playhead> playhead;
        unordered_map<int, shared_ptr<MidiClip>> clips;
        tstudio::MidiEventRegistry &midiEventRegistry;
        tstudio::MidiMsgFilter midiMsgFilter;

        // Constructors
        TrackNode(shared_ptr<AudioContext>, shared_ptr<Playhead>);
        ~TrackNode();

        // Creates a new clip in the specified scene index with option length set in bars.
        shared_ptr<MidiClip> createClip(int sceneIndex, int lengthInBars = 0);

        // Returns a clip by at the sceneIndex. If scneneIndex has no clip, will return nullptr
        shared_ptr<MidiClip> selectClip(int sceneIndex);
        void deleteClip(int sceneIndex);

        // Activates the clip at the specified sceneIndex
        void activateClip(int sceneIndex, ClipState state);

        void set_midi_device(std::string);
        void set_midi_channel(uint8_t);
        void set_instrument(shared_ptr<InstrumentDevice>);
        void onMidiMsg(MidiMsg &msg);
        void onMidiClipEvents(MidiMsg &);
    private:
        int midiHandlerId;
    };
} // namespace tstudio

#endif