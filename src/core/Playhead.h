#ifndef PLAYHEAD_H
#define PLAYHEAD_H
#include "LabSound/LabSound.h"
#include "core/SongPosition.h"
#include "library/EventRegistry.h"
#include "library/Parameter.h"
#include <functional>
#include <string>
#include <memory>
#include <map>
#include <unordered_map>
#include <utility>
using namespace lab;
using std::shared_ptr;
using std::function;
using std::string;
using std::unordered_map;
using std::pair;
namespace tstudio {

enum class LaunchQuantization {
  Off,
  Bar4,
  Bar3,
  Bar2,
  Bar,
  Beat,
  Eighth,
  Sixteenth
};

struct PlayheadTick{
  int tickCount=0;
  int sampleCount=0;
  bool precount= false;
  int precountTick= false;

};

enum class PlayheadState { PLAYING, STOPPED, RECORDING, PRECOUNT };
static unordered_map<PlayheadState, string>PlayheadStateMap = {
  {PlayheadState::PLAYING, "PLAYING"},
  {PlayheadState::STOPPED, "STOPPED"},
  {PlayheadState::RECORDING, "RECORDING"},
  {PlayheadState::PRECOUNT, "PRECOUNT"},
}; 

enum class StopMode { RETURN_TO_ZERO, RETURN_TO_PLAY, PAUSE_POSITION };

class Playhead : public FunctionNode, public EventBase {
public:
  using HandlerId = std::size_t;
  // Members
  int tpqn = 480;
  int samplesPerTick;
  bool enabled = true;
  int last_play_position_sample = 0;
  int last_play_position_tick = 0;
  StopMode stop_mode = StopMode::RETURN_TO_ZERO;
  LaunchQuantization launchQuantization = LaunchQuantization::Bar;
  pair<unsigned int, unsigned int> time_sig = pair(4,4);
  IntParam preCountBars = IntParam{"preCountBars", "Pre Count Bars", 1, "Text"};
  IntParam quantizeValue = IntParam{"quantizeValue", "quantizeValue", 16, "Text"};
  BoolParam inputQuantize = BoolParam{"inputQuantize", "inputQuantize", true, "Text"};
  BoolParam quantizeStart = BoolParam{"quantizeStart", "quantizeStart", true, "Text"};
  BoolParam quantizeEnd = BoolParam{"quantizeEnd", "quantizeEnd", false, "Text"};
  FloatParam quantizeStrength = FloatParam{"quantizeStrength", "quantizeStrength", 1.0f, "Text"};
  shared_ptr<AudioContext> audioContext;
  shared_ptr<SampledAudioNode> metronomeBeat;
  shared_ptr<AudioBus> hiHat;
  shared_ptr<AudioBus> rim;
  shared_ptr<SampledAudioNode> metronomeDownBeat;
  function<void(bool)> metronomeCallback;
  SongPosition songPosition; // TODO: Implement this for song position and in ui


  // Contructors
  Playhead(shared_ptr<AudioContext> audioContext);
  ~Playhead();

  // Methods
  void onMetronomeBeat(bool isDownBeat);
  void setMetronomeEnabled(bool);
  bool getMetronomeEnabled();
  bool toggleMetronomeEnabled();
  void setTempo(float tempo);
  float getTempo();
  bool isMod(int numerator, int denomenator);
  void setTimeSignature();
  void handleTick();
  void _generateMetronomeBeats(int &bar_tick);
  void setState(PlayheadState state);
  PlayheadState getState();
  static void callback(ContextRenderLock &r, FunctionNode *me, int channel,
                       float *buffer, int bufferSize);
  void play();
  void stop();
  void togglePlay();
  void record();
  void toggleRecord();
  bool isPlaying() const;
  bool isRecording() const;

  // Register a a function to be called when a tick event is generated. The function is called with current cumulative tick.
  HandlerId subscribeTickHandler(const function<void(PlayheadTick&)> &handler);

  // Remove handler from tick registry by id. The id was returned on
  // registration "subscribeTickHandler".
  void unsubscribeTickHandler(HandlerId id);

  // 
  

private:
  bool m_metronomeEnabled = true;
  PlayheadState _state = PlayheadState::STOPPED;
  PlayheadState _previous_state;
  atomic<HandlerId> nextHandlerId{0};
  vector<pair<HandlerId, function<void(PlayheadTick&)>>> tickHandlers;
  PlayheadTick playheadTick{};
  // Notify all handlers of current tick event.
  void notifyTick();
  int __ticksPerBeat() const;
  int __ticksPerBar() const;
  void _setSamplesPerTick() {
    samplesPerTick = (int)(60.f / _tempo / tpqn * audioContext->sampleRate());
  };
  float _tempo = 120;
};


class LaunchQuantizationHelper {
public:
  static int getQuantizeGrid(LaunchQuantization lq, int beatsPerBar ) {
    std::map<LaunchQuantization, double> multiplication_map = {
        {LaunchQuantization::Bar4, 4},
        {LaunchQuantization::Bar3, 3},
        {LaunchQuantization::Bar2, 2},
        {LaunchQuantization::Bar, 1},
        {LaunchQuantization::Beat, 0.25},
        {LaunchQuantization::Eighth, 0.125},
        {LaunchQuantization::Sixteenth, 0.0625}};
    return static_cast<int>(480 * (multiplication_map[lq] * beatsPerBar));
  }

  static double getQuantizeNumber(LaunchQuantization lq) {
    std::map<LaunchQuantization, double> multiplication_map = {
        {LaunchQuantization::Bar4, 0.25},   {LaunchQuantization::Bar3, 0.75},
        {LaunchQuantization::Bar2, 0.5},    {LaunchQuantization::Bar, 1},
        {LaunchQuantization::Beat, 4},      {LaunchQuantization::Eighth, 8},
        {LaunchQuantization::Sixteenth, 16}};
    return multiplication_map[lq];
  }
};
}

#endif // !PLAYHEAD_H