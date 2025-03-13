#include "core/Daw.h"
#include "controllers/mpk_mini_3.h"

namespace tstudio
{
    Daw::Daw(/* args */) : midiEventRegistry(MidiEventRegistry::getInstance())
    {
        midiEngine = make_shared<MidiEngine>(true);

        for (auto &[name, device] : midiEngine->inputDevices)
        {
            std::cout << name << std::endl;
        }
        midiEngine->activate();
        audioEngine = make_shared<AudioEngine>();
        audioContext = audioEngine->activate();

        // Create the playhead
        playHead = make_shared<tstudio::Playhead>(audioContext);

        // Set the playhead midi clock out handler. The playhead controls the tempo and tick counts
        playHead->setMidiClockOutHandler(std::bind(&MidiEngine::onMidiClockOut, midiEngine.get()));

        // Connect the playhead to the audio context
        audioContext->connect(audioContext->destinationNode(), playHead);

        // Create the session
        session = make_shared<Session>(audioContext, playHead);
        audioContext->connect(audioContext->destinationNode(), session->output);
        audioContext->synchronizeConnections();
    }

    Daw::~Daw()
    {
    }
    void Daw::startControllers(){
        mpk_mini_3_start(this);
    };

} // namespace tstudio



