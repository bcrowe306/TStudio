#ifndef CLIPEVENT_H
#define CLIPEVENT_H

#include <string>
#include <memory>
#include <sstream>
#include "core/MidiMsg.h"
#pragma once
namespace tstudio {

struct ClipEvent {
  int note;
  int velocity;
  int start;
  int end = 0;
  MidiMsg startMidiMsg;
  MidiMsg endMidiMsg;
  ClipEvent(){};
  ClipEvent(uint8_t note, uint8_t velocity, int start, MidiMsg startMidiMsg)
      : note(note), velocity(velocity), start(start),
        startMidiMsg(startMidiMsg) {}

  int getDuration() const { return end - start; }

  std::string toString() const {
    std::ostringstream oss;
    oss << "note: " << note << ", velocity: " << velocity
        << ", start: " << start << ", end: " << end
        << ", duration: " << getDuration();
    return oss.str();
  }
};
}

#endif // CLIPEVENT_H
