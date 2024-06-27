#ifndef SESSIONCELL_H
#define SESSIONCELL_H

#include <memory>
#include "core/MidiClip.h"
#include "app/EventRegistry.h"
#include "daw/REcordPlayOptions.h"
#pragma once
class SessionCell
{
public:
    SessionCell();
    void onLaunchTimerTick();
    bool hasClip() const;
    void stopClip();

    void setNextClipState(ClipState next_state);
    ClipState getNextClipState() const;

private:
    ClipState setClipState(ClipState next_clip_state);

    std::shared_ptr<MidiClip> clip;
    std::shared_ptr<RecordPlayOptions> record_options;
    std::shared_ptr<EventRegistry> event_registry;
    ClipState _next_clip_state;
};

#endif // SESSIONCELL_H
