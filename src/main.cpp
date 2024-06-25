#include "LabSound/LabSound.h"
#include "LabSound/core/SampledAudioNode.h"
#include "LabSound/extended/ADSRNode.h"
#include "LabSound/extended/AudioFileReader.h"
#include "bw_ap1.h"
#include "cairomm/cairomm.h"
#include "core/AudioEngine.h"
#include "core/MidiEngine.h"
#include "core/Playhead.h"
#include "core/TrackNode.h"
#include "libusb.h"
#include <iostream>
#include <memory>
#include <string>
#include <termios.h>
#include <unistd.h>

using namespace tstudio;

void set_raw_mode(termios &original) {
  termios raw = original;
  raw.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// Function to restore the terminal to its original mode
void restore_terminal_mode(const termios &original) {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

int main(int, char**){
    auto midiEngine = MidiEngine(true);
    midiEngine.activate();
    AudioEngine ae;
    auto context = ae.activate();
    auto playHead = make_shared<tstudio::Playhead>(context);
    auto track1 = TrackNode(context);
    context->connect(context->destinationNode(), playHead);
    
    // Sampler Code
    // Create nodes
    auto sampleBus = MakeBusFromFile("assets/BVKER - The Astro Perc 08.wav", context->sampleRate());
    auto samplerNode = make_shared<SampledAudioNode>(*context);

    // Configure and chain nodes
    if(sampleBus){
      
      samplerNode->setBus(sampleBus);
    }
    else{
      std::cout << "Sample not loaded\n";
    }


    // Osc
    auto oscillator = std::make_shared<OscillatorNode>(*context);
    auto adsr = make_shared<ADSRNode>(*context);
    adsr->set(.1f, 1, 1, 1, 1, .01f);
    adsr->gate()->setValueAtTime(0.f, 0.f);
    auto gain = std::make_shared<GainNode>(*context);
    gain->gain()->setValue(.5f);

    // osc -> destination
    context->connect(adsr, oscillator);
    context->connect(gain, adsr, 0, 0);
    context->connect(context->destinationNode(), gain, 0, 0);

    oscillator->frequency()->setValueAtTime(440.f,0.f);
    oscillator->setType(OscillatorType::SINE);

    // context->connect(adsr, samplerNode);
    context->connect(context->destinationNode(), samplerNode);

    context->synchronizeConnections();

    termios original;
    // Get the current terminal settings
    tcgetattr(STDIN_FILENO, &original);

    // Set the terminal to raw mode
    set_raw_mode(original);

    char c;
    while (true) {
      // Read a single character
      if (read(STDIN_FILENO, &c, 1) == -1) {
        perror("read");
        break;
      }

      // Print the ASCII value of the key pressed
      std::cout << "You pressed: " << c << " (ASCII: " << static_cast<int>(c)
                << ")\n";

      // Break the loop if 'q' is pressed
      if (c == 'p') {
        playHead->playing = !playHead->playing;
      }
      if (c == 'm') {
        playHead->enabled = !playHead->enabled;
      }
      if (c == '=') {
        playHead->setTempo(playHead->getTempo() + 1.0f);
        std::cout << playHead->getTempo();
      }
      if (c == '-') {
        playHead->setTempo(playHead->getTempo() - 1.0f);
        std::cout << playHead->getTempo();
      }
      if (c == 'k') {
        samplerNode->schedule(0.f);
      }
      if (c == 'e') {
        adsr->gate()->setValueAtTime(1.f, 0.f);
        oscillator->start(0.f);
      }
      if (c == 'r') {
        adsr->gate()->setValueAtTime(0.f, 0.f);
        // oscillator->stop(0.f);
      }
      if (c == 'q')
        break;
    }

    // Restore the original terminal settings
    restore_terminal_mode(original);

    std::cout << "Exiting...\n";
}
