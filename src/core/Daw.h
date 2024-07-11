#ifndef DAW_H
#define DAW_H

#include "LabSound/LabSound.h"
#include "core/AudioEngine.h"
#include "core/MidiEngine.h"
#include "core/MidiEventRegistry.h"
#include "core/Playhead.h"
#include "core/Session.h"
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace tstudio
{
    class Daw
    {

    public:
        MidiEventRegistry &midiEventRegistry;
        shared_ptr<MidiEngine> midiEngine;
        shared_ptr<AudioEngine> audioEngine;
        shared_ptr<AudioContext> audioContext;
        shared_ptr<Session> session;
        shared_ptr<Playhead> playHead;
        // Constructors
        Daw(/* args */);
        ~Daw();

        // Methods

        void startControllers();

    private:
        /* data */
    };

} // namespace tstudio





#endif // !DAW_