#include "core/Playhead.h"
#include "LabSound/LabSound.h"
#include "LabSound/extended/FunctionNode.h"
#include "core/TrackNode.h"
#include <iostream>
#include <memory>
#include <string>

using namespace lab;
using HandlerId = std::size_t;
namespace tstudio {

Playhead::Playhead(shared_ptr<AudioContext> audioContext)
    : FunctionNode((*audioContext)) {
  tickHandlers.reserve(4096);
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
void Playhead::setMetronomeEnabled(bool enabled){
  m_metronomeEnabled = enabled;
};

bool Playhead::toggleMetronomeEnabled(){
  m_metronomeEnabled = !m_metronomeEnabled;
  return m_metronomeEnabled;
};

bool Playhead::getMetronomeEnabled(){
  return m_metronomeEnabled;
};

void Playhead::onMetronomeBeat(bool isDownBeat) {
  if (m_metronomeEnabled)
  {
    if (isDownBeat){
      metronomeDownBeat->schedule(0.f);

    }
    else {
      metronomeBeat->schedule(0.f);
      
    }
  }
};
HandlerId Playhead::subscribeTickHandler(const function<void(PlayheadTick&)> &handler){
  HandlerId id = nextHandlerId++;
  tickHandlers.emplace_back(id, handler);
  return id;
};

void Playhead::unsubscribeTickHandler(HandlerId id){
  tickHandlers.erase(
      std::remove_if(tickHandlers.begin(), tickHandlers.end(),
                     [id](const auto &pair) { return pair.first == id; }),
      tickHandlers.end()
  );
};

void Playhead::notifyTick(){
  for (const auto &[id, handler] : tickHandlers) {
    if (handler != nullptr) {
      handler(playheadTick);
    }
  }
};

void Playhead::setState(PlayheadState state) {

  if (state == PlayheadState::RECORDING && preCountBars.get() > 0) {
    if (_state == PlayheadState::STOPPED)
    {
      state = PlayheadState::PRECOUNT;
    }
    else
    {
      state = PlayheadState::RECORDING;
    }
  }
  // Only change the state if a change is needed
  if(state != _state){
    _state = state;
    _previous_state = state;
    eventRegistry.notify("playhead.state", _state);
    auto message = "Playhead: " + PlayheadStateMap[_state] + "\n";
    LOG_TRACE(message.c_str());
  }
  
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
  setState(PlayheadState::RECORDING);
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
    playheadTick.precount = true;
    if (playheadTick.precountTick == __ticksPerBar() * preCountBars.value) {
      setState(PlayheadState::RECORDING);
      playheadTick.precount = false;
      playheadTick.precountTick = 0;
    } else {
      notifyTick();
      _generateMetronomeBeats(playheadTick.precountTick);
      playheadTick.precountTick++;
    }
    playheadTick.tickCount = 0;
    songPosition.setFromTick(playheadTick.tickCount);
  } else if (_state == PlayheadState::PLAYING ||_state == PlayheadState::RECORDING) {

    // No longer in precount, set tick to 0 and precount bool to false
    playheadTick.precountTick = 0;
    playheadTick.precount = false;

    int launch_quantization_value = LaunchQuantizationHelper::getQuantizeGrid(
        launchQuantization, time_sig.first);
    if (playheadTick.tickCount % launch_quantization_value == 0) {
      eventRegistry.notify("playhead.launch", true);
    }
    _generateMetronomeBeats(playheadTick.tickCount);

    notifyTick();
    last_play_position_tick = playheadTick.tickCount;
    playheadTick.tickCount++;
    songPosition.setFromTick(playheadTick.tickCount);
  } else if (_state == PlayheadState::STOPPED) {
    if (stop_mode == StopMode::RETURN_TO_ZERO) {
      playheadTick.sampleCount = 0;
      playheadTick.tickCount = 0;
      songPosition.setFromTick(playheadTick.tickCount);
    } else if (stop_mode == StopMode::RETURN_TO_PLAY) {
      playheadTick.sampleCount = last_play_position_sample;
      playheadTick.tickCount = last_play_position_tick;
      songPosition.setFromTick(playheadTick.tickCount);
    }
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
      if (phn->isMod(phn->playheadTick.sampleCount, phn->samplesPerTick))
        phn->handleTick();
      // notify("song_pos", song_pos);
      phn->playheadTick.sampleCount++;
      phn->last_play_position_sample = phn->playheadTick.sampleCount;
      ;
    } else {
      if (phn->stop_mode == StopMode::RETURN_TO_ZERO) {
        phn->playheadTick.sampleCount = 0;
        phn->playheadTick.tickCount = 0;
        phn->songPosition.setFromTick(phn->playheadTick.tickCount);
      } else if (phn->stop_mode == StopMode::RETURN_TO_PLAY) {
        phn->playheadTick.sampleCount = phn->last_play_position_sample;
        phn->playheadTick.tickCount = phn->last_play_position_tick;
        phn->songPosition.setFromTick(phn->playheadTick.tickCount);
      }
      phn->playheadTick.precountTick = 0;
    }
  }
}
} // namespace tstudio
