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
#include "core/MidiNode.h"
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
  const enum class ClipState {

    // Is stopped.
    STOPPED,

    // Preparing to play. If Launch quantization is set, will play at next launch event
    LAUNCHING_PLAY,

    // Preparing to record. If Launch quantization > 0, will begin record at next launch event
    LAUNCHING_RECORDING,

    // Recording with no predetermined length. In this state, clip will increase length on the fly while recording
    // If launch quantization is set, the clip length will be quantized to that value when ClipState changes.
    RECORDING_INITIAL,

    // Clip isRecording
    RECORDING,

    // Clip is playing
    PLAYING,

    // Preparing to stop. If launch quantization is set, will stop at next launch event
    STOPPING,

    // Clip is disabled
    DISABLED,
  };

  static unordered_map<ClipState, string> ClipStateMap = {
      {ClipState::STOPPED, "STOPPED"},
      {ClipState::LAUNCHING_PLAY, "LAUNCHING_PLAY"},
      {ClipState::LAUNCHING_RECORDING, "LAUNCHING_RECORDING"},
      {ClipState::RECORDING_INITIAL, "RECORDING_INITIAL"},
      {ClipState::RECORDING, "RECORDING"},
      {ClipState::PLAYING, "PLAYING"},
      {ClipState::STOPPING, "STOPPING"},
      {ClipState::DISABLED, "DISABLED"},
  };

  class MidiClip : public EventBase, public MidiNode {
  public:

    // Contructors
    MidiClip(shared_ptr<Playhead> playhead, const string &, int length_in_bars=0,
             ClipState state = ClipState::STOPPED);
    MidiClip(shared_ptr<Playhead> playhead, const string &, int trackIndex, int sceneIndex, int length_in_bars=0,
             ClipState state = ClipState::STOPPED);

    // Members
    
    unordered_map<int, vector<ClipEvent>> data;
    unordered_map<int, ClipEvent> note_map;
    // Methods
    bool receive(MidiMsg &) override;
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
    void notify(const string &, any);
    std::tuple<int, int, int> getPosition() const;
    
    void play() ;
    void record(MidiMsg &);
    void recordPrecount(MidiMsg);
    void recordEvent(ClipEvent);
    void resetLoopClip();
    void setLength();
    void setLength(int );
    void setState(ClipState );
    void setNextClipState(ClipState);
    ClipState getNextClipState() const;

    pair<int,int> getPosition();
    void setPosition(int, int);

  private:
    ClipState state = ClipState::STOPPED;
    ClipState nextState;
    int counter = 0;
    int length_in_bars;
    bool looping = true;
    int precounter = 0;
    shared_ptr<Playhead> playhead;
    float ticksPerBeat;
    int length = 0;
    int trackIndex = 0;
    int sceneIndex = 0;
    function<void(MidiMsg &)> onMidiClipOut;
    unordered_map<ClipState, function<void()>> clip_state_map = {
        {ClipState::PLAYING, bind(&MidiClip::clipPlayingState, this)},
        {ClipState::RECORDING, bind(&MidiClip::clipRecordingState, this)},
        {ClipState::LAUNCHING_PLAY, bind(&MidiClip::clipLaunchingState, this)},
        {ClipState::LAUNCHING_RECORDING,
         bind(&MidiClip::clipLaunchRecordingState, this)},
        {ClipState::STOPPED, bind(&MidiClip::clipStoppedState, this)},
        {ClipState::STOPPING, bind(&MidiClip::clipStoppingState, this)},
        {ClipState::RECORDING_INITIAL,
         bind(&MidiClip::clipInitialRecordingState, this)},
        {ClipState::DISABLED, bind(&MidiClip::clipDisabledState, this)},
    };

    // Methods
    void quantizeClipLength(ClipState );
    void init();
    void onLaunchEvent(any);
    ClipState processClipState(ClipState);
    void onPlayheadStateChange(any);
    void onPrecountTick(/*SongPos song_pos*/);
    void onTimeSigChange(const pair<int, int> &);
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
