#ifndef CLIPEVENT_H
#define CLIPEVENT_H

#include <string>
#include <memory>
#include <sstream>
#pragma once

struct ClipEvent
{
    int note;
    int velocity;
    int start;
    int end = 0;
    std::shared_ptr<MidiMsg> startMidiMsg = nullptr;
    std::shared_ptr<MidiMsg> endMidiMsg = nullptr;

    ClipEvent(int note, int velocity, int start, int end = 0, std::shared_ptr<MidiMsg> startMidiMsg = nullptr, std::shared_ptr<MidiMsg> endMidiMsg = nullptr)
        : note(note), velocity(velocity), start(start), end(end), startMidiMsg(startMidiMsg), endMidiMsg(endMidiMsg) {}

    int getDuration() const
    {
        return end - start;
    }

    std::string toString() const
    {
        std::ostringstream oss;
        oss << "note: " << note
            << ", velocity: " << velocity
            << ", start: " << start
            << ", end: " << end
            << ", duration: " << getDuration();
        return oss.str();
    }
};

#endif // CLIPEVENT_H
