#include "daw/SessionCell.h"

SessionCell::SessionCell() : _next_clip_state(ClipState::STOPPED)
{
    record_options = std::make_shared<RecordPlayOptions>();
    event_registry = std::make_shared<EventRegistry>();
    event_registry->subscribe("playhead.launch", [this]()
                              { onLaunchTimerTick(); });
}

void SessionCell::onLaunchTimerTick()
{
    if (clip && clip->getState() != _next_clip_state)
    {
        clip->setState(_next_clip_state);
    }
}

bool SessionCell::hasClip() const
{
    return clip != nullptr;
}

void SessionCell::stopClip()
{
    if (clip)
    {
        clip->setState(ClipState::STOPPING);
        _next_clip_state = ClipState::STOPPED;
    }
}

void SessionCell::setNextClipState(ClipState next_state)
{
    if (next_state != ClipState::NONE)
    {
        auto next_state_modified = setClipState(next_state);
        if (record_options->getLaunchQuantize() == LaunchQuantization::Off)
        {
            clip->setState(next_state_modified);
            _next_clip_state = next_state_modified;
        }
        else
        {
            if (next_state == ClipState::RECORDING && clip->getState() == ClipState::PLAYING)
            {
                _next_clip_state = next_state_modified;
                clip->setState(next_state_modified);
            }
            else
            {
                _next_clip_state = next_state_modified;
            }
        }
    }
    else
    {
        _next_clip_state = ClipState::NONE;
    }
}

ClipState SessionCell::getNextClipState() const
{
    return _next_clip_state;
}

ClipState SessionCell::setClipState(ClipState next_clip_state)
{
    if (next_clip_state == ClipState::RECORDING)
    {
        switch (clip->getState())
        {
        case ClipState::NEWRECORDING:
            return ClipState::INITIALRECORDING;
        case ClipState::LAUNCHRECORDING:
            return ClipState::RECORDING;
        default:
            return next_clip_state;
        }
    }
    else
    {
        return next_clip_state;
    }
}
