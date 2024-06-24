#include "LabSound/LabSound.h"
#include "libusb.h"
#include "bw_ap1.h"
#include "cairomm/cairomm.h"
#include "core/MidiEngine.h"
#include "core/AudioEngine.h"
#include "core/Playhead.h"
#include <memory>
#include <string>
#include <iostream>


using namespace tstudio;
int main(int, char**){
    auto midiEngine = MidiEngine(true);
    midiEngine.activate();
    AudioEngine ae;
    auto context = ae.activate();
    auto playHead = make_shared<tstudio::Playhead>(context);
    context->connect(context->destinationNode(), playHead);
    context->synchronizeConnections();
    playHead->start(0.f);
    std::string msg;
    std::getline(std::cin, msg);

}
