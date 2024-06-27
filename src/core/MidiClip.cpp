#include "core/MidiClip.h"
#include <iostream>


MidiClip::MidiClip(const std::string &name, int length_in_bars, std::pair<int, int> time_sig)
    : name(name), length_in_bars(length_in_bars), time_sig(time_sig), tick_counter(0),
      precount_tick_counter(0), looping(true), playhead_state(PlayheadState::STOPPED), state(ClipState::STOPPED),
      tpqn(480), ticksPerBeat(tpqn / (time_sig.second / 4)), length(0)
{

    event_registry = std::make_shared<EventRegistry>();
    record_options = std::make_shared<RecordPlayOptions>();

    clip_state_map = {
        {ClipState::PLAYING, std::bind(&MidiClip::clipPlayingState, this)},
        {ClipState::RECORDING, std::bind(&MidiClip::clipRecordingState, this)},
        {ClipState::LAUNCHING, std::bind(&MidiClip::clipLaunchingState, this)},
        {ClipState::NEWRECORDING, std::bind(&MidiClip::clipNewRecordingState, this)},
        {ClipState::LAUNCHRECORDING, std::bind(&MidiClip::clipLaunchRecordingState, this)},
        {ClipState::STOPPED, std::bind(&MidiClip::clipStoppedState, this)},
        {ClipState::STOPPING, std::bind(&MidiClip::clipStoppingState, this)},
        {ClipState::INITIALRECORDING, std::bind(&MidiClip::clipInitialRecordingState, this)},
        {ClipState::DISABLED, std::bind(&MidiClip::clipDisabledState, this)},
    };

    setLength();

    event_registry->subscribe("playhead.tick", [this](/*SongPos song_pos*/)
                              { incTickCounter(/*song_pos*/); });
    event_registry->subscribe("playhead.state", [this](PlayheadState state)
                              { onPlayheadStateChange(state); });
    event_registry->subscribe("playhead.precount_tick", [this](/*SongPos song_pos*/)
                              { onPrecountTick(/*song_pos*/); });
}

void MidiClip::notify(const std::string &event_id, /*additional args*/)
{
    event_registry->notifyListeners("clip." + event_id, /*additional args*/);
}

void MidiClip::resetLoopClip()
{
    if (tick_counter == length && looping)
    {
        tick_counter = 0;
    }
}

void MidiClip::clipPlayingState()
{
    play();
    tick_counter += 1;
    resetLoopClip();
}

void MidiClip::clipInitialRecordingState()
{
    increaseLength();
    play();
    tick_counter += 1;
}

void MidiClip::clipRecordingState()
{
    play();
    tick_counter += 1;
    resetLoopClip();
}

void MidiClip::clipLaunchingState()
{
    tick_counter = 0;
}

void MidiClip::clipDisabledState()
{
    tick_counter = 0;
}

void MidiClip::clipLaunchRecordingState()
{
    tick_counter = 0;
}

void MidiClip::clipNewRecordingState()
{
    tick_counter = 0;
}

void MidiClip::clipStoppingState()
{
    play();
    tick_counter += 1;
}

void MidiClip::clipStoppedState()
{
    tick_counter = 0;
}

void MidiClip::setMidiOutCallback(std::function<void(MidiMsg)> callback)
{
    onMidiClipOut = callback;
}

void MidiClip::onPlayheadStateChange(PlayheadState state)
{
    playhead_state = state;
    if (state == PlayheadState::STOPPED)
    {
        tick_counter = 0;
    }
}

void MidiClip::incTickCounter(/*SongPos song_pos*/)
{
    auto stateHandler = clip_state_map.find(state);
    if (stateHandler != clip_state_map.end())
    {
        stateHandler->second();
    }
}

void MidiClip::onPrecountTick(/*SongPos song_pos*/)
{
    precount_tick_counter += 1;
}

std::tuple<int, int, int> MidiClip::getPosition() const
{
    int bar = tick_counter / (ticksPerBeat * time_sig.first);
    int beat = (tick_counter / ticksPerBeat) % time_sig.first;
    int tick = (tick_counter % tpqn) / 120;
    return std::make_tuple(bar + 1, beat + 1, tick + 1);
}

void MidiClip::setLength()
{
    if (length_in_bars > 0)
    {
        length = length_in_bars * time_sig.first * tpqn;
    }
}

void MidiClip::increaseLength()
{
    length = tick_counter + 2;
}

std::pair<int, int> MidiClip::getTimeSig() const
{
    return time_sig;
}

void MidiClip::setTimeSig(const std::pair<int, int> &time_sig)
{
    this->time_sig = time_sig;
    ticksPerBeat = tpqn / (time_sig.second / 4);
}

void MidiClip::onMidiInEvent(const MidiMsg &event)
{
    if (state == ClipState::RECORDING || state == ClipState::INITIALRECORDING)
    {
        record(event);
    }
    if (state == ClipState::NEWRECORDING || state == ClipState::LAUNCHRECORDING)
    {
        recordPrecount(event);
    }
}

void MidiClip::recordPrecount(const MidiMsg &event)
{
    int precount_total_ticks = record_options->getPreCountBars() * tpqn;
    if (precount_tick_counter > precount_total_ticks - 120)
    {
        bool on = event.status == MidiBytes::NOTE_ON && event.data2 > 0;
        bool off = event.status == MidiBytes::NOTE_OFF || (event.status == MidiBytes::NOTE_ON && event.data2 == 0);
        if (on)
        {
            note_map[event.data1] = ClipEvent(event.data1, event.data2, 0, event);
        }
        if (off)
        {
            auto &clipevent = note_map[event.data1];
            clipevent.end = tick_counter;
            clipevent.endMidiMsg = event;
            recordEvent(clipevent);
        }
    }
}

void MidiClip::record(const MidiMsg &event)
{
    bool on = event.status == MidiBytes::NOTE_ON && event.data2 > 0;
    bool off = event.status == MidiBytes::NOTE_OFF || (event.status == MidiBytes::NOTE_ON && event.data2 == 0);
    if (on)
    {
        note_map[event.data1] = ClipEvent(event.data1, event.data2, tick_counter, event);
    }
    if (off)
    {
        auto &clipevent = note_map[event.data1];
        clipevent.end = tick_counter;
        clipevent.endMidiMsg = event;
        recordEvent(clipevent);
    }
}

void MidiClip::recordEvent(const ClipEvent &event)
{
    if (record_options->isInputQuantize())
    {
        if (record_options->isQuantizeStart())
        {
            event.start = Quantize(event.start, record_options->getQuantizeValue(), record_options->getQuantizeStrength());
        }
        if (record_options->isQuantizeEnd())
        {
            event.end = Quantize(event.end, record_options->getQuantizeValue(), record_options->getQuantizeStrength());
        }
    }
    data[event.note].push_back(event);
}

void MidiClip::play()
{
    notify("position", /*pass position args*/);
    for (auto &pair : data)
    {
        for (auto &e : pair.second)
        {
            if (e.start == tick_counter)
            {
                sendMidiOut(e.startMidiMsg);
            }
            if (e.end == tick_counter)
            {
                sendMidiOut(e.endMidiMsg);
            }
        }
    }
}

void MidiClip::sendMidiOut(const MidiMsg &event)
{
    auto ev = event;
    ev.direction = MIDIDirection::OUT;
    if (onMidiClipOut)
    {
        onMidiClipOut(ev);
    }
}

ClipState MidiClip::getState() const
{
    return state;
}

void MidiClip::quantizeClipLength(ClipState state)
{
    if (state == ClipState::INITIALRECORDING && this->state != ClipState::INITIALRECORDING)
    {
        tick_counter = 0;
        if (record_options->getLaunchQuantize() != LaunchQuantization::Off)
        {
            length = Quantize(length, LaunchQuantization::getQuantizeNumber(record_options->getLaunchQuantize()), 1.0, tpqn);
        }
    }
}

void MidiClip::setState(ClipState state)
{
    if (state != ClipState::NONE)
    {
        quantizeClipLength(state);
        this->state = state;
        std::cout << name << ": " << state << " " << length / (480 * 4) << std::endl;
        notify("state", /*pass state args*/);
    }
}

int MidiClip::getLength() const
{
    return length;
}

void MidiClip::setLength(int length)
{
    this->length = length;
    notify("length", /*pass length args*/);
}

float MidiClip::getLengthInBars() const
{
    return static_cast<float>(length) / (tpqn * time_sig.first);
}
