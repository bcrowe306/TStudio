#include "core/TrackNode.h"
#include "core/MidiMsg.h"
#include "core/MidiNode.h"
#include "library/UUID_Gen.h"

using namespace lab;
using namespace placeholders;
using std::make_shared;
using std::shared_ptr;
using std::string;

namespace tstudio {
  TrackNode::TrackNode(shared_ptr<AudioContext> context) : context(context), midiEventRegistry(MidiEventRegistry::getInstance()), id(GenerateUUID()) {

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
    this->addOutputNode(instrument);
    this->context->connect(output, this->instrument->output);
  }
  void TrackNode::onMidiMsg(MidiMsg &msg) {
    this->pushIn(msg);
    if (arm.get()) {
      this->push(msg);
    }
  }
  void TrackNode::onMidiClipEvents(MidiMsg &event){
    if(instrument){
      this->push(event);
    }
  }
}; // namespace tstudio