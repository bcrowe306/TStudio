
#ifndef MPK_MINI_3_H
#define MPK_MINI_3_H

#include "core/Daw.h"
#include "core/MidiMsgFilter.h"
#include "core/MidiMsg.h"

namespace tstudio {
    void mpk_mini_3_start(Daw *daw) {
        auto filter = MidiMsgFilter{"MPK mini 3", 0};
        daw->midiEventRegistry.subscribe(filter, [&](MidiMsg &event) {
            char knob1 = 70;
            char knob2 = 71;
            char knob3 = 72;
            char knob4 = 73;
            char knob5 = 74;
            char knob6 = 75;
            char knob7 = 76;
            char knob8 = 77;

            if (event.getNoteNumber().note == knob1 && event.getVelocity() == 1) {
            daw->session->nextTrack();
            }
            if (event.getNoteNumber().note == knob1 && event.getVelocity() == 127) {
            daw->session->prevTrack();
            }
            if (event.getNoteNumber().note == knob2 && event.getVelocity() == 1) {
            daw->session->nextScene();
            }
            if (event.getNoteNumber().note == knob2 && event.getVelocity() == 127) {
            daw->session->prevScene();
            }

            if (event.getNoteNumber().note == knob3 && event.getVelocity() == 1) {
            daw->playHead->setTempo(daw->playHead->getTempo() + 1.0f);
            }
            if (event.getNoteNumber().note == knob3 && event.getVelocity() == 127) {
            daw->playHead->setTempo(daw->playHead->getTempo() - 1.0f);
            } 
        });
    }
}

#endif // !MPK_MINI_3_H