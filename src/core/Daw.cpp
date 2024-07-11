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
        playHead = make_shared<tstudio::Playhead>(audioContext);
        audioContext->connect(audioContext->destinationNode(), playHead);
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



