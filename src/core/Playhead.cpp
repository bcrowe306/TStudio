#include "core/Playhead.h"
#include "LabSound/LabSound.h"
#include "LabSound/extended/FunctionNode.h"
#include "core/TrackNode.h"
#include <iostream>
#include <memory>
#include <string>

using namespace lab;

namespace tstudio {

Playhead::Playhead(shared_ptr<AudioContext> audioContext)
    : FunctionNode((*audioContext)) {
  this->audioContext = audioContext;
  _setSamplesPerTick();
  metronomeBeat = std::make_shared<SampledAudioNode>((*audioContext));
  metronomeBeat->schedule(0);
  metronomeDownBeat = std::make_shared<SampledAudioNode>((*audioContext));
  metronomeDownBeat->schedule(0);

  hiHat = MakeBusFromFile("assets/BVKER - The Astro Perc 10.wav", false,
                          audioContext->sampleRate());
  rim = MakeBusFromFile("assets/BVKER - The Astro Perc 08.wav", false,
                        audioContext->sampleRate());
  metronomeBeat->setBus(hiHat);
  metronomeBeat->initialize();
  metronomeDownBeat->setBus(rim);
  metronomeDownBeat->initialize();

  audioContext->connect(audioContext->destinationNode(), metronomeBeat, 0, 0);
  audioContext->connect(audioContext->destinationNode(), metronomeDownBeat, 0,
                        0);
  this->setFunction(callback);
  this->start(0.f);
};

Playhead ::~Playhead(){};

void Playhead::onMetronomeBeat(bool isDownBeat) {
  if (enabled) {
    if (isDownBeat){
      metronomeDownBeat->schedule(0.f);

    }
    else {
      metronomeBeat->schedule(0.f);
      
    }
  }
};

void Playhead::setState(PlayheadState state) {
  if (state == PlayheadState::RECORDING && preCountBars.get() > 0) {
    if (_previous_state == PlayheadState::PRECOUNT) {
      state = PlayheadState::RECORDING;
    } else if (_previous_state == PlayheadState::PLAYING) {
      state = PlayheadState::RECORDING;
    } else {
      state = PlayheadState::PRECOUNT;
    }
  }
  _state = state;
  _previous_state = state;
  eventRegistry.notify("playhead.state", _state);
  std::cout << PlayheadStateMap[_state] << std::endl;
}
void Playhead::play() { setState(PlayheadState::PLAYING); }

void Playhead::togglePlay() {
  if (isPlaying()) {
    stop();
  } else {
    play();
  }
};

void Playhead::toggleRecord() {
  if (isRecording()) {
    play();
  } else {
    record();
  }
};

void Playhead::stop() { setState(PlayheadState::STOPPED); }

void Playhead::record() {
  if (preCountBars.get() > 0 and _state != PlayheadState::PLAYING) {
    setState(PlayheadState::PRECOUNT);
  } else {
    setState(PlayheadState::RECORDING);
  }
}

bool Playhead::isPlaying() const {
  return _state == PlayheadState::PLAYING || _state == PlayheadState::RECORDING;
}

bool Playhead::isRecording() const {
  return _state == PlayheadState::RECORDING ||
         _state == PlayheadState::PRECOUNT;
}

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
PlayheadState Playhead::getState() { return _state; }
void Playhead::handleTick() {
  
  if (_state == PlayheadState::PRECOUNT) {
    if (precount_bar_tick == __ticksPerBar() * preCountBars.value) {
      setState(PlayheadState::RECORDING);
      precount_bar_tick = 0;
    } else {
      eventRegistry.notify("playhead.precount_tick", precount_bar_tick);
      _generateMetronomeBeats(precount_bar_tick);
      precount_bar_tick++;
    }
    ticks = 0;
  } else if (_state == PlayheadState::PLAYING ||_state == PlayheadState::RECORDING) {
    
    int launch_quantization_value = LaunchQuantizationHelper::getQuantizeGrid(
        launchQuantization, time_sig.first);
    if (ticks % launch_quantization_value == 0) {
      eventRegistry.notify("playhead.launch", true);
    }
    _generateMetronomeBeats(ticks);
    eventRegistry.notify("playhead.tick", ticks);
    last_play_position_tick = ticks;
    ticks++;
  } else if (_state == PlayheadState::STOPPED) {
    if (stop_mode == StopMode::RETURN_TO_ZERO) {
      counter = 0;
      ticks = 0;
    } else if (stop_mode == StopMode::RETURN_TO_PLAY) {
      counter = last_play_position_sample;
      ticks = last_play_position_tick;
    }
    precount_bar_tick = 0;
  }
}
void Playhead::_generateMetronomeBeats(int &bar_tick) {
  int ticks_per_beat = __ticksPerBeat();
  if (isMod(bar_tick, ticks_per_beat)) {
    bool isDownBeat = isMod(bar_tick, ticks_per_beat * time_sig.first);
    onMetronomeBeat(isDownBeat);
    eventRegistry.notify("playhead.metronome", isDownBeat);
  }
}
int Playhead::__ticksPerBeat() const { return tpqn / (time_sig.first / 4); }

int Playhead::__ticksPerBar() const {
  return __ticksPerBeat() * time_sig.first;
}
void Playhead::callback(ContextRenderLock &r, FunctionNode *me, int channel,
                        float *buffer, int bufferSize) {
  auto phn = (Playhead *)me;

  for (size_t i = 0; i < bufferSize; i++) {
    //  istick
    if (phn->getState() != PlayheadState::STOPPED) {
      if (phn->isMod(phn->counter, phn->samplesPerTick))
        phn->handleTick();
      // notify("song_pos", song_pos);
      phn->counter++;
      phn->last_play_position_sample = phn->counter;
      ;
    } else {
      if (phn->stop_mode == StopMode::RETURN_TO_ZERO) {
        phn->counter = 0;
        phn->ticks = 0;
      } else if (phn->stop_mode == StopMode::RETURN_TO_PLAY) {
        phn->counter = phn->last_play_position_sample;
        phn->ticks = phn->last_play_position_tick;
      }
      phn->precount_bar_tick = 0;
    }
  }
}
} // namespace tstudio
