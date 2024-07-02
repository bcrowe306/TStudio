#include "LabSound/LabSound.h"
#include "bw_ap1.h"
#include "cairomm/cairomm.h"
#include "core/AudioEngine.h"
#include "core/MidiClip.h"
#include "core/MidiEngine.h"
#include "core/MidiEventRegistry.h"
#include "core/MidiMsg.h"
#include "core/MidiMsgFilter.h"
#include "core/Playhead.h"
#include "library/ScrollView.h"
#include "core/Session.h"
#include "libusb.h"
#include <__functional/bind_front.h>
#include <cstdlib>
#include <array>
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
#define OUT std::cout 
#define END std::endl


using namespace tstudio;
using namespace placeholders;
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

  // Daw Setup Code
  MidiEventRegistry &mer = MidiEventRegistry::getInstance();
  auto midiEngine = MidiEngine(true);

  for(auto &[name, device] : midiEngine.inputDevices){
    std::cout << name << std::endl;
  }
  midiEngine.activate();
  AudioEngine ae;
  auto context = ae.activate();
  auto playHead = make_shared<tstudio::Playhead>(context);
  context->connect(context->destinationNode(), playHead);

  auto session = make_shared<Session>(context, playHead);
  context->connect(context->destinationNode(), session->output);
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

  auto filter = MidiMsgFilter{"MPK mini 3", 0};
  mer.subscribe(filter, [&](MidiMsg &event)
                {
    char knob1 = 70;
    char knob2 = 71;
    char knob3 = 72;
    char knob4 = 73;
    char knob5 = 74;
    char knob6 = 75;
    char knob7 = 76;
    char knob8 = 77;

    if(event.getNoteNumber().note == knob1 && event.getVelocity() == 1){
      session->nextTrack();
    }
    if(event.getNoteNumber().note == knob1 && event.getVelocity() == 127){
      session->prevTrack();
    }
    if(event.getNoteNumber().note == knob2 && event.getVelocity() == 1){
      session->nextScene();
    }
    if(event.getNoteNumber().note == knob2 && event.getVelocity() == 127){
      session->prevScene();
    }

    if(event.getNoteNumber().note == knob3 && event.getVelocity() == 1){
      playHead->setTempo(playHead->getTempo() + 1.0f);
      std::cout << playHead->getTempo() << std::endl;
    }
    if(event.getNoteNumber().note == knob3 && event.getVelocity() == 127){
      playHead->setTempo(playHead->getTempo() - 1.0f);
      std::cout << playHead->getTempo() << std::endl;
    } });
  char c;
  std::array<char, 2> C;
  while (true) {
    clear_screen(original);
    // Print the ASCII value of the key pressed
    std::cout << "You pressed: " << c << " (ASCII: " << static_cast<int>(c)
              << ")\n";
    // for (size_t i = sv.start; i <= sv.end; i++) {
    //   auto &vec = sv.get_vec();
    //   if (i == sv.get_cursor()) {
    //     std::cout << CYAN;
    //     std::cout << dir[i].path().filename().string() << std::endl;
    //     std::cout << WHITE;
    //   } else {
    //     std::cout << dir[i].path().filename().string() << std::endl;
    //   }
    // }
    // Read a single character
    int sceneIndex = 0;
    for (auto scene: session->scenes){
      int trackIndex = 0;
      OUT << " | ";
      for (auto track : session->tracks){
        if (session->selectedTrackIndex() == trackIndex && session->selectedSceneIndex() == sceneIndex)
          OUT << CYAN;
        else
          OUT << WHITE;
          auto clip = session->selectClipByPosition(std::pair<int, int>{trackIndex, sceneIndex});
          if (clip != nullptr)
          {
            OUT << clip->name.value << " | ";
          }
          else
          {
            OUT << "-----------" << " | ";
          }
          OUT << WHITE;
          trackIndex++;
        }
      OUT << scene.name.value << " |" << END;
      sceneIndex++;
    }

    // Line Separator
    OUT << " * ";
    for (auto track : session->tracks)
    {
      OUT << "-----------" << " * ";
    }
    OUT << END;
    
    // Track Row
    OUT << " | ";
    int trackIndex = 0;
    for (auto track : session->tracks)
    {
        if(session->selectedTrackIndex() == trackIndex){
          OUT << CYAN << track->name.value <<  WHITE <<" | " ;
        }else{

          OUT << track->name.value <<  " | " ;
        }
        trackIndex++;
    }
    OUT << END;

    if (read(STDIN_FILENO, &c, 1) == -1) {
      perror("read");
      break;
    }
  
    // Break the loop if 'q' is pressed
    if (c == '.') {
      sv.increment();
    }
    if (c == 'c') {
      session->newClip(2);
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
      if(playHead->getState() == PlayheadState::PLAYING){
      }else{
      }
    }
    if (c == 'r') {
      playHead->toggleRecord();
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
    if (c == 't') {
      session->addTrack();
    }
    if (c == 'x') {
      session->nextTrack();
    }
    if (c == 'z') {
      session->prevTrack();
    }
    if (c == 'w') {
      session->prevScene();
    }
    if (c == 's') {
      session->nextScene();
    }
    if (c == 'd') {
      session->deleteClip();
    }

    if (c == 'q')
      break;
  }

  // Restore the original terminal settings
  restore_terminal_mode(original);

  std::cout << "Exiting...\n";
}
