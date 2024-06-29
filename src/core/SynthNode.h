#ifndef SYNTHNODE_H
#define SYNTHNODE_H

#include "LabSound//LabSound.h"
#include "LabSound/core/AudioNode.h"
#include "LabSound/core/GainNode.h"
#include "LabSound/core/OscillatorNode.h"
#include "LabSound/extended/ADSRNode.h"
#include "core/MidiEventRegistry.h"
#include "core/MidiMsg.h"
#include "core/MidiMsgFilter.h"
#include <functional>
#include <memory>
#include <string>
using std::make_shared;
using std::shared_ptr;
using namespace lab;
using namespace placeholders;

namespace tstudio {

class SynthNode {
public:

  bool enabled = false;
  shared_ptr<AudioContext> audioContext;
  shared_ptr<OscillatorNode> oscillator;
  shared_ptr<ADSRNode> adsr;
  shared_ptr<GainNode> gain;
  tstudio::MidiEventRegistry &midiEventRegistry;
  tstudio::MidiMsgFilter midiMsgFilter;
  SynthNode(shared_ptr<AudioContext> context)
      : midiEventRegistry(tstudio::MidiEventRegistry::getInstance()),
        midiMsgFilter(tstudio::MidiMsgFilter{"all", 0}) {
    // Osc
    audioContext = context;
    oscillator = make_shared<OscillatorNode>(*audioContext);
    adsr = make_shared<ADSRNode>(*audioContext);
    adsr->set(.1f, 1, 10.f, 5000.f, 0.f, .01f);
    adsr->gate()->setValueAtTime(0.f, 0.f);
    gain = make_shared<GainNode>(*audioContext);
    gain->gain()->setValue(.5f);

    // osc -> destination
    audioContext->connect(adsr, oscillator);
    audioContext->connect(gain, adsr, 0, 0);
    audioContext->connect(audioContext->destinationNode(), gain, 0, 0);
    midiEventRegistry.subscribe(midiMsgFilter,
                                std::bind(&SynthNode::onMidiMsg, this, _1));
  }

  void onMidiMsg(tstudio::MidiMsg msg) {
    if(enabled){
      auto freq = msg.getNoteNumber().getFrequency();
      if (msg.isNoteOn()) {
        oscillator->frequency()->setValueAtTime(freq, 0.f);
        oscillator->start(0.f);
        adsr->gate()->setValueAtTime(1.f, 0.f);
      }
      if (msg.isNoteOff()) {
        adsr->gate()->setValueAtTime(0.f, 0.f);
      }
    }
    
  }
};
}; // namespace tstudio

#endif // !SYNTHNODE_H#de