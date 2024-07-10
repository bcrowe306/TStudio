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
  TrackNode::TrackNode(shared_ptr<AudioContext> context, shared_ptr<Playhead> playhead) : context(context), midiEventRegistry(MidiEventRegistry::getInstance()), id(GenerateUUID()) {

            // Setup midi stuff
            this->playhead = playhead;
            midiMsgFilter = MidiMsgFilter{"all", 0};
            midiHandlerId = midiEventRegistry.subscribe(
                midiMsgFilter, std::bind(&TrackNode::onMidiMsg, this, _1));
            volumeNode = make_shared<GainNode>(*this->context);
            panNode = make_shared<StereoPannerNode>(*this->context);
            input = make_shared<AnalyserNode>(*context);
            output = make_shared<AnalyserNode>(*context);
            this->context->connect(panNode, input);
            this->context->connect(volumeNode, panNode);
            this->context->connect(output, volumeNode);
        }

  TrackNode::~TrackNode(){
    midiEventRegistry.unsubscribe(midiHandlerId);
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
  shared_ptr<MidiClip> TrackNode::selectClip(int sceneIndex)
  {
    if(clips.find(sceneIndex) != clips.end()){
      return clips.at(sceneIndex);
    }
    else
    {
      return shared_ptr<MidiClip>(nullptr);
    }
  };
  shared_ptr<MidiClip> TrackNode::createClip(int sceneIndex, int lengthInBars)
  {

    auto clipNumber = std::to_string(++clipCount);
    auto clipName = this->name.value + " " + clipNumber;
    if(clips.find(sceneIndex) == clips.end()){
      auto it = clips.emplace(sceneIndex, make_shared<MidiClip>(this->playhead, clipName, lengthInBars));
      if(it.second){
        auto clip = it.first->second;
        clip->addOutputNode(shared_from_this());
        clip->color.set(this->color.value);
        return clip;
      }
      // TODO: Implement event notification on clip creation.
      // eventRegistry.notify("session.clip_create", m_selectedClipIndex);
    }
    {
      
    }
    return shared_ptr<MidiClip>(nullptr);
  };
  void TrackNode::deleteClip(int sceneIndex){
    if(clips.find(sceneIndex) != clips.end()){
      auto clip = clips.at(sceneIndex);
      clip->removeOutputNode(shared_from_this());
      clips.erase(sceneIndex);
    }
  }
  void TrackNode::activateClip(int sceneIndex, ClipState state)
  {
    for(auto &[index, clip] : this->clips){
      if(index == sceneIndex){
        clip->setNextClipState(state);
      }else{
        clip->setNextClipState(ClipState::STOPPED);
      }
    }
  }
}; // namespace tstudio