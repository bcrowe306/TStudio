#include "core/TrackNode.h"
#include "core/MidiMsg.h"

using namespace lab;
using namespace placeholders;
using std::make_shared;
using std::shared_ptr;
using std::string;

namespace tstudio {
  TrackNode::TrackNode(shared_ptr<AudioContext> context) : context(context), midiEventRegistry(MidiEventRegistry::getInstance()) {

            // Setup midi stuff
            midiMsgFilter = MidiMsgFilter{"all", 0};
            midiEventRegistry.subscribe(
                midiMsgFilter, std::bind(&TrackNode::onMidiMsg, this, _1));
            volumeNode = make_shared<GainNode>(*this->context);
            panNode = make_shared<StereoPannerNode>(*this->context);
            input = make_shared<AnalyserNode>(*context);
            output = make_shared<AnalyserNode>(*context);
            this->context->connect(panNode, input);
            this->context->connect(volumeNode, panNode);
            this->context->connect(output, volumeNode);
        }
  void TrackNode::set_midi_device(std::string device) { midiMsgFilter.device = device; }
  void TrackNode::set_midi_channel(uint8_t channel) {
    midiMsgFilter.channel = channel;
  }
  void TrackNode::set_instrument(shared_ptr<InstrumentDevice> instrument) {
    this->instrument = instrument;
    this->context->connect(output, this->instrument->output);
  }
  void TrackNode::onMidiMsg(MidiMsg &msg) {

    if (instrument.get() != nullptr && arm.get()) {
      instrument->onMidiMsg(msg);
    }
    for(auto clip : clips){
      if(clip.get()){
        clip->midiInEvent(msg);
      }
    }
  }
  void TrackNode::onMidiClipEvents(MidiMsg &event){
    if(instrument){
      instrument->onMidiMsg(event);
    }
  }
}; // namespace tstudio