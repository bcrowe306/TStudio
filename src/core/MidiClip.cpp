#include "core/MidiClip.h"
#include "core/Playhead.h"
#include <iostream>
#include <charconv>
#include <utility>

using namespace placeholders;

namespace tstudio {

MidiClip::MidiClip(shared_ptr<Playhead> playhead, string name,
                   int length_in_bars, ClipState state)
    :state(state), playhead(playhead) 
  {
    this->name.set(name);
    init(length_in_bars);
};
MidiClip::MidiClip(shared_ptr<Playhead> playhead,  string name, int trackIndex, int sceneIndex,
                   int length_in_bars, ClipState state)
    : state(state), 
    playhead(playhead), trackIndex(trackIndex), sceneIndex(sceneIndex)
  {
    this->name.set(name);
    init(length_in_bars);
};

void MidiClip::deInit(){
  eventRegistry.unsubscribe("playhead.state", playheadStateHandlerId);
  eventRegistry.unsubscribe("playhead.launch", playheadLaunchHandlerId);
  playhead->unsubscribeTickHandler(handlerId);
}

MidiClip::~MidiClip(){
  deInit();
}

void MidiClip::init(int bars) {
  launchEventHandlers.reserve(64);
  songPosition.time_sig = playhead->time_sig;
  songPosition.tpqn = playhead->tpqn;
  setLengthInBars(bars);
  handlerId = playhead->subscribeTickHandler(std::bind(&MidiClip::incTickCounter, this, _1));
  playheadStateHandlerId = eventRegistry.subscribe("playhead.state", bind(&MidiClip::onPlayheadStateChange, this, _1));
  playheadLaunchHandlerId = eventRegistry.subscribe("playhead.launch", bind(&MidiClip::onLaunchEvent, this, _1));
}

void MidiClip::onLaunchEvent(any data){
  if(state != nextState){
    setState(nextState);
  }
}

void MidiClip::setNextClipState(ClipState next_state)
{
  if (next_state != this->state){
    switch (next_state)
    {
    case ClipState::STOPPED:
      if (playhead->launchQuantization == LaunchQuantization::Off)
      {
        setState(next_state);
      }
      else
      {
        setState(ClipState::STOPPING);
        nextState = next_state;
      }
      break;
    case ClipState::RECORDING:
      if (length == 0)
      {
        setState(ClipState::LAUNCHING_RECORDING);
        nextState = ClipState::RECORDING_INITIAL;
      }
      else
      {
        if (getState() == ClipState::PLAYING)
        {
          setState(ClipState::RECORDING);
        }
        else
        {
          setState(ClipState::LAUNCHING_RECORDING);
          nextState = ClipState::RECORDING;
        }
      }
      break;
    case ClipState::PLAYING:
      if (playhead->launchQuantization == LaunchQuantization::Off)
      {
        setState(ClipState::PLAYING);
      }
      else
      {
        setState(ClipState::LAUNCHING_PLAY);
        nextState = ClipState::PLAYING;
      }
    default:
      break;
    }
  }
  
}

ClipState MidiClip::processClipState(ClipState nextClipState)
{
  return nextClipState;
}

ClipState MidiClip::getNextClipState() const {
  return nextState;
}

void MidiClip::notify(const std::string &event_id, any data) {
  eventRegistry.notify("clip." + event_id, data);
}

void MidiClip::resetLoopClip() {
  if (counter == length && looping) {
    counter = 0;
  }
}

void MidiClip::setLoopStatus(bool isLooping) {
  looping = isLooping;
}

bool MidiClip::getLoopStatus() {
  return looping;
}

void MidiClip::clipPlayingState() {
  play();
  counter += 1;
  resetLoopClip();
}

void MidiClip::clipInitialRecordingState() {
  increaseLength();
  play();
  counter += 1;
}

void MidiClip::clipRecordingState() {
  play();
  counter += 1;
  resetLoopClip();
}
pair<int, int> MidiClip::getPosition(){
  return pair<int, int>{trackIndex, sceneIndex};
};

int MidiClip::getLength(){
  return this->length;
};

int MidiClip::getCounter(){
  return this->counter;
};

std::pair<int, int> MidiClip::getNoteRange() {
  if (data.empty()) {
    return std::pair<int, int>{0, 0}; // Return (0, 0) if the map is empty
  }

  int minKey = std::numeric_limits<int>::max();
  int maxKey = std::numeric_limits<int>::min();

  // Find the minimum and maximum keys
  for (const auto &pair : data) {
    minKey = std::min(minKey, pair.first);
    maxKey = std::max(maxKey, pair.first);
  }

  // Return the difference between maxKey and minKey
  return std::pair<int, int>{minKey,maxKey};
};

float MidiClip::playheadPosPercentage(){
  return (float)counter / (float)length;
};

void MidiClip::setPosition(int trackIndex, int sceneIndex){
  this->trackIndex = trackIndex;
  this->sceneIndex = sceneIndex;
};

void MidiClip::setTimeSig(pair<int, int> time_sig){
  songPosition.time_sig = time_sig;
};
void MidiClip::clipLaunchingState() { counter = 0; }

void MidiClip::clipDisabledState() { counter = 0; }

void MidiClip::clipLaunchRecordingState() { counter = 0; }

void MidiClip::clipNewRecordingState() { counter = 0; }

void MidiClip::clipStoppingState() {
  play();
  counter += 1;
}

void MidiClip::clipStoppedState() { counter = 0; }

void MidiClip::onPlayheadStateChange(any data) {
  auto state = std::any_cast<PlayheadState>(data);
  auto currentClipState = getState();
  // if (playhead->getState() == PlayheadState::STOPPED) {
  //   if (currentClipState == ClipState::RECORDING_INITIAL){
  //     setState(ClipState::PLAYING);
  //   }
  //   counter = 0;
  // }
  // else if(state == PlayheadState::PLAYING){
  //   if (currentClipState == ClipState::RECORDING || currentClipState == ClipState::RECORDING_INITIAL){
  //     setNextClipState(ClipState::PLAYING);
  //   }
  // }
}

void MidiClip::incTickCounter(PlayheadTick &playheadTick) {
  if(playheadTick.precount){
    onPrecountTick();
  }else{
    auto stateHandler = clip_state_map.find(state);
    if (stateHandler != clip_state_map.end()) {
      stateHandler->second();
    }
  }
  
}

void MidiClip::onPrecountTick(/*SongPos song_pos*/) { precounter += 1; }

std::tuple<int, int, int> MidiClip::getPosition() const {
  int bar = counter / (ticksPerBeat * playhead->time_sig.first);
  int beat = (int)(counter / ticksPerBeat) % playhead->time_sig.first;
  int tick = (counter % playhead->tpqn) / 120;
  return std::make_tuple(bar + 1, beat + 1, tick + 1);
}

void MidiClip::setLength(int length) { 
  this->length = length;
  songPosition.setFromTick(length); 
  }

void MidiClip::setLengthInBars(int bars) {
  if (bars > 0)
  {
    songPosition.bar = bars;
  }
  auto lengthInTick = songPosition.getTickCount();
  setLength(lengthInTick);
}

void MidiClip::increaseLength() { length = counter + 2; }


void MidiClip::onTimeSigChange(const pair<int, int> &time_sig) {
  ticksPerBeat = playhead->tpqn / (playhead->time_sig.second / 4);
}
bool MidiClip::receive(MidiMsg &msg) {
  if (state == ClipState::RECORDING || state == ClipState::RECORDING_INITIAL || state == ClipState::LAUNCHING_RECORDING)
  {
    record(msg);
  }
}

void MidiClip::record(MidiMsg &event) {
  auto playheadState = playhead->getState();

  // Quantize start note values entered in during precount
  if (playheadState == PlayheadState::PRECOUNT){
    int precount_total_ticks = playhead->preCountBars.get() * playhead->tpqn;
    if (precounter > precount_total_ticks - 120)
    {
      auto note = event.getNoteNumber().note;
      if (event.isNoteOn())
      {
        note_map[(int)note] = ClipEvent(note, event.getVelocity(), 0, event);
      }
      if (event.isNoteOff())
      {
        auto &clipevent = note_map[(int)event.getNoteNumber().note];
        clipevent.end = counter;
        clipevent.endMidiMsg = event;
        recordEvent(clipevent);
      }
    }
  }

  // regular record
  else if (playheadState == PlayheadState::RECORDING){
    if (event.isNoteOn())
      {
        note_map[event.getNoteNumber()] = ClipEvent(
            event.getNoteNumber().note, event.getVelocity(), counter, event);
      }
      if (event.isNoteOff())
      {
        auto &clipevent = note_map[event.getNoteNumber().note];
        clipevent.end = counter;
        clipevent.endMidiMsg = event;
        recordEvent(clipevent);
      }
  }
}

void MidiClip::recordEvent(ClipEvent event) {
  if (playhead->inputQuantize.value) {
    if (playhead->quantizeStart.value) {
      event.start = Quantize(event.start, playhead->quantizeValue.value,
                             playhead->quantizeStrength.value);
    }
    if (playhead->quantizeEnd.value) {
      event.end = Quantize(event.end, playhead->quantizeValue.value,
                           playhead->quantizeStrength.value);
    }
  }
  data[event.note].push_back(event);
}

void MidiClip::play() {
  // notify("position", getPosition());
  
  for (auto &pair : data) {
    for (auto &e : pair.second) {
      if (e.start == counter) {
        this->push(e.startMidiMsg);
      }
      if (e.end == counter) {
        this->push(e.endMidiMsg);
      }
    }
  }
}


ClipState MidiClip::getState() const { return state; }

void MidiClip::quantizeClipLength() {
  if (playhead->launchQuantization != LaunchQuantization::Off) {
    setLength(Quantize(length,
                       LaunchQuantizationHelper::getQuantizeNumber(
                           playhead->launchQuantization),
                       1.0f));
  }
}

void MidiClip::setState(ClipState newState) {
  
  // If we're trying to set to RECORDING state and length is zero, we set to initial recording
  if(length == 0 && newState == ClipState::RECORDING){
      newState = ClipState::RECORDING_INITIAL;
  }

  // If this was the initial recording, quantize length according to launch quantization
  if (this->state == ClipState::RECORDING_INITIAL && newState != ClipState::RECORDING_INITIAL)
  {
    LOG_TRACE("MidiClip: quantize length");
    counter = 0;
    quantizeClipLength();
  }
  auto message = "MidiClip: " + name.value + " " + ClipStateMap[newState] + "\n";
  LOG_TRACE(message.c_str());
  this->state = newState;
  this->nextState = newState;
  notify("state", ClipStateMap[newState]);
}

int MidiClip::getLength() const { return length; }



float MidiClip::getLengthInBars() const {
  return static_cast<float>(length) / (playhead->tpqn * playhead->time_sig.first);
}
}

