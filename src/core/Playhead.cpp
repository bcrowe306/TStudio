#include "LabSound/LabSound.h"
#include "core/Playhead.h"
#include <iostream>
#include <memory>
using namespace lab;

namespace tstudio {

  Playhead::Playhead(shared_ptr<AudioContext> audioContext) : FunctionNode((*audioContext)) {
    this->audioContext = audioContext;
    _setSamplesPerTick();
    metronomeBeat = std::make_shared<SampledAudioNode>((*audioContext));
    metronomeDownBeat = std::make_shared<SampledAudioNode>((*audioContext));

    hiHat = MakeBusFromFile(
        "/Users/brandoncrowe/Documents/Git/TStudio/assets/BVKER - The Astro Perc 10.wav", false,
        audioContext->sampleRate());
    rim = MakeBusFromFile(
        "/Users/brandoncrowe/Documents/Git/TStudio/assets/BVKER - The Astro Perc 08.wav", false,
        audioContext->sampleRate());
    metronomeBeat->setBus(hiHat);

    metronomeDownBeat->setBus(rim);

    audioContext->connect(  audioContext->destinationNode() , metronomeBeat,0, 0);
    audioContext->connect(audioContext->destinationNode(), metronomeDownBeat, 0,
                          0);
    this->setFunction(callback);
  };


  Playhead ::~Playhead(){};


  void Playhead::onMetronomeBeat(bool isDownBeat) {
    if (enabled) {
      if (isDownBeat)
        metronomeDownBeat->schedule(0.f);
      else
        metronomeBeat->schedule(0.f);
    }
  };

  void Playhead::setTempo(float tempo) {
    _tempo = tempo;
    this->_setSamplesPerTick();
  };


  float Playhead::getTempo() { return _tempo; };

  bool Playhead::isMod(int numerator, int denomenator) {
    if (numerator < denomenator) {
      return numerator == 0;
    }
    return numerator % denomenator == 0;
  };

  void Playhead::setTimeSignature(){};

  void Playhead::callback(ContextRenderLock &r, FunctionNode *me,
                                     int channel, float *buffer,
                                     int bufferSize) {
    auto playHeadNode = (Playhead *)me;

    for (size_t i = 0; i < bufferSize; i++) {
      //  istick
      if (playHeadNode->isMod(playHeadNode->counter,
                              (int)playHeadNode->samplesPerTick)) {

        // isMetronomebeat
        if (playHeadNode->isMod(playHeadNode->ticks, playHeadNode->tpqn) &&
            playHeadNode->playing) {
          bool isDownBeat = playHeadNode->isMod(playHeadNode->ticks,
                                                playHeadNode->tpqn *
                                                    playHeadNode->beatsPerBar);
          playHeadNode->onMetronomeBeat(isDownBeat);
        }

        // Only tick if playing is true
        if (playHeadNode->playing)
          playHeadNode->ticks++;
        else {
          playHeadNode->ticks = 0;
        }
      }
      playHeadNode->counter++;
    }
  }
}
