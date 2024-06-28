#ifndef MIDICLIP_H
#define MIDICLIP_H
#include "core/Playhead.h"
#include <any>
#include <unordered_map>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <memory>
#include <utility>
#include "core/ClipEvent.h"
#include "core/MidiMsg.h"
#include "library/EventRegistry.h"

using std::shared_ptr;
using std::string;
using std::make_shared;
using std::function;
using std::bind;
using std::pair;
using std::unordered_map;
using std::map;
using std::vector;
using std::any;
using std::any_cast;

namespace tstudio {
  enum class ClipState {
    STOPPING,
    STOPPED,
    LAUNCHING,
    LAUNCHRECORDING,
    NEWRECORDING,
    RECORDING,
    PLAYING,
    DISABLED,
    INITIALRECORDING
  };

  static unordered_map<ClipState, string> ClipStateMap = {
    {ClipState::STOPPING, "STOPPING"},
    {ClipState::STOPPED, "STOPPED"},
    {ClipState::LAUNCHING, "LAUNCHING"},
    {ClipState::LAUNCHRECORDING, "LAUNCHRECORDING"},
    {ClipState::NEWRECORDING, "NEWRECORDING"},
    {ClipState::RECORDING, "RECORDING"},
    {ClipState::PLAYING, "PLAYING"},
    {ClipState::DISABLED, "DISABLED"},
    {ClipState::INITIALRECORDING, "INITIALRECORDING"},
  };

  

  class MidiClip : public EventBase {
  public:
    unordered_map<int, vector<ClipEvent>> data;
    unordered_map<int, ClipEvent> note_map;
    string name;

    // Contructors
    MidiClip(shared_ptr<Playhead> playhead, const string &, int length_in_bars=0,
             ClipState state = ClipState::STOPPED);

    // Methods
    void clipPlayingState();
    void clipInitialRecordingState();
    void clipRecordingState();
    void clipLaunchingState();
    void clipDisabledState();
    void clipLaunchRecordingState();
    void clipNewRecordingState();
    void clipStoppingState();
    void clipStoppedState();
    ClipState getState() const;
    void increaseLength();
    void incTickCounter(/*SongPos song_pos*/);
    int getLength() const;
    float getLengthInBars() const;
    void midiInEvent(MidiMsg &);
    void notify(const string &, any);
    void onPlayheadStateChange(any );
    std::tuple<int, int, int> getPosition() const;
    void onPrecountTick(/*SongPos song_pos*/);
    void onTimeSigChange(const pair<int, int> &);
    void play();
    void record(MidiMsg &);
    void recordPrecount(MidiMsg);
    void recordEvent(ClipEvent);
    void resetLoopClip();
    void setLength();
    void setLength(int );
    void sendMidiOut(MidiMsg &);
    void setMidiOutCallback(std::function<void(MidiMsg &)> );
    void setState(ClipState );

  private:

    ClipState state = ClipState::STOPPED;
    int counter = 0;
    int length_in_bars;
    bool looping = true;
    int precounter = 0;
    shared_ptr<Playhead> playhead;
    float ticksPerBeat;
    int length;
    function<void(MidiMsg &)> onMidiClipOut;
    unordered_map<ClipState, function<void()>> clip_state_map = {
        {ClipState::PLAYING, bind(&MidiClip::clipPlayingState, this)},
        {ClipState::RECORDING, bind(&MidiClip::clipRecordingState, this)},
        {ClipState::LAUNCHING, bind(&MidiClip::clipLaunchingState, this)},
        {ClipState::NEWRECORDING,
         bind(&MidiClip::clipNewRecordingState, this)},
        {ClipState::LAUNCHRECORDING,
         bind(&MidiClip::clipLaunchRecordingState, this)},
        {ClipState::STOPPED, bind(&MidiClip::clipStoppedState, this)},
        {ClipState::STOPPING, bind(&MidiClip::clipStoppingState, this)},
        {ClipState::INITIALRECORDING,
         bind(&MidiClip::clipInitialRecordingState, this)},
        {ClipState::DISABLED, bind(&MidiClip::clipDisabledState, this)},
    };
    void quantizeClipLength(ClipState );
  };

}

static int Quantize(int delta, int quantize_value = 16, float strength = 1.0,
                    int tpqn = 480) {
    float resolution = 1.0 / (quantize_value / 4.0);
    int grid_tick_size = std::round(tpqn * resolution);
    int quantized_delta =
        std::round(static_cast<float>(delta) / grid_tick_size) * grid_tick_size;
    int note_spread = quantized_delta - delta;
    int shift_amount = std::round(note_spread * strength);
    int new_delta = shift_amount + delta;
    return new_delta;
}

#endif // MIDICLIP_H
