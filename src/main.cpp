#include "LabSound/LabSound.h"
#include "bw_ap1.h"
#include "cairomm/cairomm.h"
#include "core/AudioEngine.h"
#include "core/InstrumentDevice.h"
#include "core/MidiEngine.h"
#include "core/MidiEventRegistry.h"
#include "core/MidiMsg.h"
#include "core/MidiMsgFilter.h"
#include "core/Playhead.h"
#include "core/SamplerNode.h"
#include "core/SynthNode.h"
#include "core/TrackNode.h"
#include "library/ScrollView.h"
#include "libusb.h"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <sys/termios.h>
#include <termios.h>
#include <unistd.h>
#include <vector>
#define CYAN "\x1B[36m"
#define WHITE "\x1B[37m"
using namespace tstudio;
using std::getenv;
void set_raw_mode(termios &original) {
  termios raw = original;
  raw.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void clear_screen(termios &term) { std::cout << "\x1B[2J"; }

// Function to restore the terminal to its original mode
void restore_terminal_mode(const termios &original) {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

int main(int, char **) {
  MidiEventRegistry &mer = MidiEventRegistry::getInstance();
  auto midiEngine = MidiEngine(true);
  midiEngine.activate();
  AudioEngine ae;
  auto context = ae.activate();
  auto playHead = make_shared<tstudio::Playhead>(context);
  auto track1 = TrackNode(context);
  context->connect(context->destinationNode(), playHead);
  auto synth = make_shared<SynthNode>(context);
  auto sample = make_shared<tstudio::SamplerNode>(
      context, "assets/BVKER - The Astro Perc 08.wav");

  // Tracks vector
  vector<shared_ptr<TrackNode>> tracks;

  track1.set_instrument(sample);
  context->connect(context->destinationNode(), track1.output);
  context->synchronizeConnections();

  termios original;
  // Get the current terminal settings
  tcgetattr(STDIN_FILENO, &original);

  // Set the terminal to raw mode
  set_raw_mode(original);

  namespace fs = std::filesystem;
  auto HOME = (std::string)getenv("HOME");
  fs::path fileDir = HOME + "/Documents";
  std::vector<filesystem::directory_entry> dir;
  ScrollView sv(dir);

  for (auto &entry : fs::directory_iterator(fileDir)) {
    dir.emplace_back(entry);
  }
  sv.render_view();

  char c;
  while (true) {
    clear_screen(original);
    // Print the ASCII value of the key pressed
    std::cout << tracks.size() << std::endl;
    std::cout << "You pressed: " << c << " (ASCII: " << static_cast<int>(c)
              << ")\n";
    for (size_t i = sv.start; i <= sv.end; i++) {
      auto &vec = sv.get_vec();
      if (i == sv.get_cursor()) {
        std::cout << CYAN;
        std::cout << dir[i].path().filename().string() << std::endl;
        std::cout << WHITE;
      } else {
        std::cout << dir[i].path().filename().string() << std::endl;
      }
    }
    // Read a single character
    if (read(STDIN_FILENO, &c, 1) == -1) {
      perror("read");
      break;
    }

    // Break the loop if 'q' is pressed
    if (c == '.') {
      sv.increment();
    }
    if (c == ',') {
      sv.decrement();
    }
    if (c == '>') {
      if (dir[sv.get_cursor()].is_directory()) {
        auto new_dir = dir[sv.get_cursor()];
        dir.clear();
        for (auto &entry : fs::directory_iterator(new_dir.path())) {
          dir.emplace_back(entry);
        }
        sv.render_view(true);
      }
    }

    if (c == '<') {
      auto entryItem = dir[sv.get_cursor()];
      if (entryItem.path().has_parent_path()) {
        auto parentPath = entryItem.path().parent_path();
        if (parentPath.has_parent_path()) {
          dir.clear();
          for (auto &entry : fs::directory_iterator(parentPath.parent_path())) {
            dir.emplace_back(entry);
          }
          sv.render_view(true);
        }
      }
    }

    if (c == 'p') {
      playHead->togglePlay();
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
      sample->sampledAudioNode->schedule(0,.01,0);
    }
    if (c == 't') {
      auto t = tracks.emplace_back(make_shared<tstudio::TrackNode>(context));
      t->set_instrument(make_shared<tstudio::SamplerNode>(
          context, "assets/BVKER - The Astro Perc 08.wav"));
      context->connect(context->destinationNode(), t->output);
      
    }

    if (c == 'q')
      break;
  }

  // Restore the original terminal settings
  restore_terminal_mode(original);

  std::cout << "Exiting...\n";
}
