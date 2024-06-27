#ifndef MIDICLIP_H
#define MIDICLIP_H
#pragma once
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <memory>
#include <utility>
#include "core/ClipEvent.h"
#include "library/EventRegistry.h"
#include "core/MidiMsg.h"
#include "daw/RecordPlayOptions.h"
#include "daw/AppStates.h"

class MidiClip
{
public:
    MidiClip(const std::string &name, int length_in_bars = 0, std::pair<int, int> time_sig = {4, 4});
    template <class... Types>
    void notify(const std::string &event_id, Types &...args);
    void resetLoopClip();

    void clipPlayingState();
    void clipInitialRecordingState();
    void clipRecordingState();
    void clipLaunchingState();
    void clipDisabledState();
    void clipLaunchRecordingState();
    void clipNewRecordingState();
    void clipStoppingState();
    void clipStoppedState();

    void setMidiOutCallback(std::function<void(MidiMsg)> callback);

    void onPlayheadStateChange(PlayheadState state);
    void incTickCounter(/*SongPos song_pos*/);
    void onPrecountTick(/*SongPos song_pos*/);

    std::tuple<int, int, int> getPosition() const;
    void setLength();
    void increaseLength();

    std::pair<int, int> getTimeSig() const;
    void setTimeSig(const std::pair<int, int> &time_sig);

    void onMidiInEvent(const MidiMsg &event);
    void recordPrecount(const MidiMsg &event);
    void record(const MidiMsg &event);
    void recordEvent(const ClipEvent &event);

    void play();
    void sendMidiOut(const MidiMsg &event);

    ClipState getState() const;
    void setState(ClipState state);

    int getLength() const;
    void setLength(int length);

    float getLengthInBars() const;

private:
    void quantizeClipLength(ClipState state);

    std::string name;
    int length_in_bars;
    std::pair<int, int> time_sig;
    int tick_counter;
    int precount_tick_counter;
    bool looping;
    std::shared_ptr<EventRegistry> event_registry;
    PlayheadState playhead_state;
    ClipState state;
    int tpqn;
    std::map<int, std::vector<ClipEvent>> data;
    std::shared_ptr<RecordPlayOptions> record_options;
    std::map<int, ClipEvent> note_map;
    float ticksPerBeat;
    std::function<void(MidiMsg)> onMidiClipOut;
    int length;
    std::map<ClipState, std::function<void()>> clip_state_map;
};

#endif // MIDICLIP_H
