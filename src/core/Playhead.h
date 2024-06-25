#ifndef PLAYHEAD_H
#define PLAYHEAD_H
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/core/AudioNode.h"
#include "LabSound/core/SampledAudioNode.h"
#include "LabSound/extended/AudioContextLock.h"
#include "LabSound/extended/AudioFileReader.h"
#include "LabSound/extended/FunctionNode.h"
#include <memory>
using namespace lab;
using std::shared_ptr;
namespace tstudio {

class Playhead : public FunctionNode {
public:
  shared_ptr<AudioContext> audioContext;
  int tpqn = 480;
  int counter = 0;
  int ticks = 0;
  int samplesPerTick;
  bool enabled = true;
  bool playing = false;
  unsigned int beatsPerBar = 4;
  unsigned int beatValue = 4;
  shared_ptr<SampledAudioNode> metronomeBeat;
  shared_ptr<AudioBus> hiHat;
  shared_ptr<AudioBus> rim;
  shared_ptr<SampledAudioNode> metronomeDownBeat;

  Playhead(shared_ptr<AudioContext> audioContext);
  ~Playhead();
  void onMetronomeBeat(bool isDownBeat);
  void setTempo(float tempo);
  float getTempo();
  bool isMod(int numerator, int denomenator);
  void setTimeSignature();
  static void callback(ContextRenderLock &r, FunctionNode *me, int channel,
                       float *buffer, int bufferSize);

private:
  void _setSamplesPerTick() {
    samplesPerTick = (int)(60.f / _tempo / tpqn * audioContext->sampleRate());
  };
  float _tempo = 120;
};
}

#endif // !PLAYHEAD_H