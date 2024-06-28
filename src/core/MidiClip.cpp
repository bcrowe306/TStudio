#include "core/MidiClip.h"
#include "core/Playhead.h"
#include <iostream>
#include <charconv>

using namespace placeholders;

namespace tstudio {

MidiClip::MidiClip(shared_ptr<Playhead> playhead, const string &name,
                   int length_in_bars, ClipState state)
    : name(name), length_in_bars(length_in_bars), state(state), playhead(playhead) {

  setLength();

  eventRegistry.subscribe("playhead.tick", [this](any data) {
    incTickCounter();
  });
  eventRegistry.subscribe("playhead.state",  bind(&MidiClip::onPlayheadStateChange, this, _1));
  eventRegistry.subscribe(
      "playhead.precount_tick",
      [this](any data) { onPrecountTick(/*song_pos*/); });
};
void MidiClip::notify(const std::string &event_id, any data) {
  eventRegistry.notify("clip." + event_id, data);
}

void MidiClip::resetLoopClip() {
  if (counter == length && looping) {
    counter = 0;
  }
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

void MidiClip::clipLaunchingState() { counter = 0; }

void MidiClip::clipDisabledState() { counter = 0; }

void MidiClip::clipLaunchRecordingState() { counter = 0; }

void MidiClip::clipNewRecordingState() { counter = 0; }

void MidiClip::clipStoppingState() {
  play();
  counter += 1;
}

void MidiClip::clipStoppedState() { counter = 0; }

void MidiClip::setMidiOutCallback(std::function<void(MidiMsg &)> callback) {
  onMidiClipOut = callback;
}

void MidiClip::onPlayheadStateChange(any data) {
  auto state = std::any_cast<PlayheadState>(data);
  if (playhead->getState() == PlayheadState::STOPPED) {
    counter = 0;
  }
}

void MidiClip::incTickCounter(/*SongPos song_pos*/) {
  auto stateHandler = clip_state_map.find(state);
  if (stateHandler != clip_state_map.end()) {
    stateHandler->second();
  }
}

void MidiClip::onPrecountTick(/*SongPos song_pos*/) { precounter += 1; }

std::tuple<int, int, int> MidiClip::getPosition() const {
  int bar = counter / (ticksPerBeat * playhead->time_sig.first);
  int beat = (int)(counter / ticksPerBeat) % playhead->time_sig.first;
  int tick = (counter % playhead->tpqn) / 120;
  return std::make_tuple(bar + 1, beat + 1, tick + 1);
}

void MidiClip::setLength() {
  if (length_in_bars > 0) {
    length = length_in_bars * playhead->time_sig.first * playhead->tpqn;
  }
}

void MidiClip::increaseLength() { length = counter + 2; }


void MidiClip::onTimeSigChange(const pair<int, int> &time_sig) {
  ticksPerBeat = playhead->tpqn / (playhead->time_sig.second / 4);
}

void MidiClip::midiInEvent(MidiMsg &event) {
  if (state == ClipState::RECORDING || state == ClipState::INITIALRECORDING) {
    record(event);
  }
  if (state == ClipState::NEWRECORDING || state == ClipState::LAUNCHRECORDING) {
    recordPrecount(event);
  }
}

void MidiClip::recordPrecount(MidiMsg event) {
  int precount_total_ticks = playhead->preCountBars.get() * playhead->tpqn;
  if (precounter > precount_total_ticks - 120) {
    auto note = event.getNoteNumber().note;
    if (event.isNoteOn()) {
      note_map[(int)note] = ClipEvent(note, event.getVelocity(), 0,event);
    }
    if (event.isNoteOff()) {
      auto &clipevent = note_map[(int)event.getNoteNumber().note];
      clipevent.end = counter;
      clipevent.endMidiMsg = event;
      recordEvent(clipevent);
    }
  }
}

void MidiClip::record(MidiMsg &event) {
 
  if (event.isNoteOn()) {
    note_map[event.getNoteNumber()] = ClipEvent(
        event.getNoteNumber().note, event.getVelocity(), counter, event);
  }
  if (event.isNoteOff()) {
    auto &clipevent = note_map[event.getNoteNumber().note];
    clipevent.end = counter;
    clipevent.endMidiMsg = event;
    recordEvent(clipevent);
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
        std::cout << counter << std::endl;
        sendMidiOut(e.startMidiMsg);
      }
      if (e.end == counter) {
        sendMidiOut(e.endMidiMsg);
      }
    }
  }
}

void MidiClip::sendMidiOut(MidiMsg &event) {
  if (onMidiClipOut) {
    onMidiClipOut(event);
  }
}

ClipState MidiClip::getState() const { return state; }

void MidiClip::quantizeClipLength(ClipState state) {
  if (state == ClipState::INITIALRECORDING &&
      this->state != ClipState::INITIALRECORDING) {
    counter = 0;

      std::cout << "IM HERE!" << std::endl;
    if (playhead->launchQuantization != LaunchQuantization::Off) {
      length = Quantize(
          length,
          LaunchQuantizationHelper::getQuantizeNumber(playhead->launchQuantization),
          1.0f);
      std::array<char, 10> str;
      auto [ptr, ec] = std::to_chars(str.data(), str.data() + str.size(), length);
    }
  }
}

void MidiClip::setState(ClipState state) {
    quantizeClipLength(state);
    this->state = state;
    std::cout << name << ": " << ClipStateMap[state] << " " << length 
              << std::endl;
    notify("state", ClipStateMap[state]);
}

int MidiClip::getLength() const { return length; }

void MidiClip::setLength(int length) {
  this->length = length;
  notify("length", length);
}

float MidiClip::getLengthInBars() const {
  return static_cast<float>(length) / (playhead->tpqn * playhead->time_sig.first);
}
}

