#ifndef SAMPLERNODE_H
#define SAMPLERNODE_H

#include "LabSound//LabSound.h"
#include "LabSound/core/AnalyserNode.h"
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioNode.h"
#include "LabSound/core/GainNode.h"
#include "LabSound/core/OscillatorNode.h"
#include "LabSound/core/SampledAudioNode.h"
#include "LabSound/extended/ADSRNode.h"
#include "LabSound/extended/AudioFileReader.h"
#include "core/InstrumentDevice.h"
#include <functional>
#include <iostream>
#include <memory>
#include <string>
using std::make_shared;
using std::shared_ptr;
using namespace lab;
using namespace placeholders;

namespace tstudio {

class SamplerNode : public InstrumentDevice {
public:
  std::string audioFilePath;
  shared_ptr<AudioContext> audioContext;
  shared_ptr<AudioBus> audioBus;
  shared_ptr<SampledAudioNode> sampledAudioNode;
  shared_ptr<ADSRNode> adsr;
  shared_ptr<GainNode> gain;
  SamplerNode(shared_ptr<AudioContext> context, std::string audioFilePath) :
        audioFilePath(audioFilePath), InstrumentDevice(context) 
  {
    audioContext = context;
    output = make_shared<AnalyserNode>(*context);
    sampledAudioNode = make_shared<SampledAudioNode>(*audioContext);
    sampledAudioNode->schedule(0);
    loadAudioFile(audioFilePath);

    // TODO: Implement ADSR for this node type
    // adsr = make_shared<ADSRNode>(*audioContext);
    // adsr->set(.1f, 1, 10.f, 5000.f, 0.f, .01f);
    // adsr->gate()->setValueAtTime(0.f, 0.f);
    gain = make_shared<GainNode>(*audioContext);
    gain->gain()->setValue(.5f);

    // osc -> destination
    audioContext->connect(gain, sampledAudioNode);
    audioContext->connect(output, gain, 0, 0);
  }

  void loadAudioFile(std::string filePath){
    audioBus = MakeBusFromFile(this->audioFilePath.c_str(), false);
    
    if (audioBus) {
      sampledAudioNode->setBus(audioBus);
    }else
    {
      std::cout << "Audio File: " << filePath << "Failed to load\n";
    }
  }


  void onMidiMsg(tstudio::MidiMsg &msg) override {
    auto freq = msg.getNoteNumber().getFrequency();
    if (msg.isNoteOn()) {
      
      gain->gain()->setValueAtTime(msg.getUnitVelocity(),  0.f);
      sampledAudioNode->schedule(0.f);
    }
    if (msg.isNoteOff()) {
      // sampledAudioNode->clearSchedules();
    }
  }
};
}; // namespace tstudio

#endif // SAMPLERNODE_H