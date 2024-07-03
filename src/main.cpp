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
#include <vector>
#include "hello_imgui/hello_imgui.h"
#include "library/UUID_Gen.h"
#define CYAN "\x1B[36m"
#define WHITE "\x1B[37m"
#define OUT std::cout
#define END std::endl

const char s8_zero = 0, s8_one = 1, s8_fifty = 50, s8_min = -128, s8_max = 127;
const ImU8 u8_zero = 0, u8_one = 1, u8_fifty = 50, u8_min = 0, u8_max = 255;
const short s16_zero = 0, s16_one = 1, s16_fifty = 50, s16_min = -32768, s16_max = 32767;
const ImU16 u16_zero = 0, u16_one = 1, u16_fifty = 50, u16_min = 0, u16_max = 65535;
const ImS32 s32_zero = 0, s32_one = 1, s32_fifty = 50, s32_min = INT_MIN / 2, s32_max = INT_MAX / 2, s32_hi_a = INT_MAX / 2 - 100, s32_hi_b = INT_MAX / 2;
const ImU32 u32_zero = 0, u32_one = 1, u32_fifty = 50, u32_min = 0, u32_max = UINT_MAX / 2, u32_hi_a = UINT_MAX / 2 - 100, u32_hi_b = UINT_MAX / 2;
const ImS64 s64_zero = 0, s64_one = 1, s64_fifty = 50, s64_min = LLONG_MIN / 2, s64_max = LLONG_MAX / 2, s64_hi_a = LLONG_MAX / 2 - 100, s64_hi_b = LLONG_MAX / 2;
const ImU64 u64_zero = 0, u64_one = 1, u64_fifty = 50, u64_min = 0, u64_max = ULLONG_MAX / 2, u64_hi_a = ULLONG_MAX / 2 - 100, u64_hi_b = ULLONG_MAX / 2;
const float f32_zero = 0.f, f32_one = 1.f, f32_lo_a = -10000000000.0f, f32_hi_a = +10000000000.0f;
const double f64_zero = 0., f64_one = 1., f64_lo_a = -1000000000000000.0, f64_hi_a = +1000000000000000.0;

// State
static char s8_v = 127;
static ImU8 u8_v = 255;
static short s16_v = 32767;
static ImU16 u16_v = 65535;
static ImS32 s32_v = -1;
static ImU32 u32_v = (ImU32)-1;
static ImS64 s64_v = -1;
static ImU64 u64_v = (ImU64)-1;
static float f32_v = 0.123f;
static double f64_v = 90000.01234567890123456789;

using namespace tstudio;
using namespace placeholders;
using std::getenv;

void ui(shared_ptr<Session> session, shared_ptr<Playhead> playhead)
{
  HelloImGui::Run([playhead, session]()
                  {
    static std::pair<int, int> sessionPosition;
    auto tempo = playhead->getTempo();
    ImGui::AlignTextToFramePadding();
    ImGui::PushItemWidth(100.f);
    auto tempoText = ImGui::InputScalar("Tempo", ImGuiDataType_Float, &tempo, true ? &f32_one : NULL);
    if(tempoText){
      if(tempo >30 && tempo < 300){
        playhead->setTempo(tempo);
      }
    }
    ImGui::SameLine();

    std::string buttonLabel;
    float playButtonHue = 0.35f;
    switch (playhead->getState())
    {
    case PlayheadState::RECORDING:
      buttonLabel = "Recording";
      playButtonHue = 0.35f;
      break;
    case PlayheadState::PRECOUNT:
      buttonLabel = "Precount";
      playButtonHue = 0.35f;
      break;
    case PlayheadState::PLAYING:
      buttonLabel = "Record";
      playButtonHue = 0.35f;
      break;
    case PlayheadState::STOPPED:
      buttonLabel = "Record";
      playButtonHue = 0.66;
      break;
    default:
      buttonLabel = "Record";
      playButtonHue = 0.66;
      break;
    }

    // Playbutton
    // ImGui::PushID("playButton");
    // ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(playButtonHue, 0.6f, 0.6f));
    // ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(playButtonHue / 7.0f, 0.7f, 0.7f));
    // ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(playButtonHue / 7.0f, 0.8f, 0.8f));
    auto playButton = ImGui::Button(playhead->isPlaying() ? "Stop" : "Play");ImGui::SameLine();
    if(playButton){
      playhead->togglePlay();
    }
    // ImGui::PopID();
    ImGui::SameLine();

    // RecordButton
    float buttonHue = playhead->isRecording() ? 0.0f : 0.5f;
    ImGui::PushID("recordButton");
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(buttonHue, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(buttonHue / 7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(buttonHue / 7.0f, 0.8f, 0.8f));
    
    auto recordButton = ImGui::Button(buttonLabel.c_str());
    if(recordButton){
      playhead->toggleRecord();

    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();

    ImGui::SameLine();
    ImGui::PushID("stopButton");
    auto stopButton = ImGui::Button("Stop");
    if(stopButton){
      playhead->stop();
    }
    ImGui::PopID();
    ImGui::SameLine();

    // addTrackButton
    ImGui::PushID("addTrackButton");
    auto addTrackButton = ImGui::Button("Add Track");
    if(addTrackButton){
      session->addTrack();
    }
    ImGui::PopID();
    ImGui::SameLine();

    // addSceneButton
    ImGui::PushID("addSceneButton");
    auto addSceneButton = ImGui::Button("Add Scene");
    if(addSceneButton){
      session->addScene();
    }
    ImGui::PopID();

    ImGui::SameLine();
    // Metronome
    static bool check = playhead->getMetronomeEnabled();
    auto metronomeCheckbox = ImGui::Checkbox("Metronome", &check);
    ImGui::SameLine();
    if (metronomeCheckbox)
    {
      playhead->toggleMetronomeEnabled();
    }

    
    // positionText
    ImGui::PushID("positionText");
    auto positionString = (std::to_string(sessionPosition.first) + " : " + std::to_string(sessionPosition.second)).c_str();
    ImGui::Text(positionString);
    ImGui::PopID();

    ImGui::Separator();
    // Tracks Header
    for (int trackIndex = 0; trackIndex < session->tracks.size(); trackIndex++)
    {
      auto track = session->getTrackByIndex(trackIndex);
      if(track != nullptr){
        ImGui::Selectable(track->name.value.c_str(), session->selectedTrackIndex() == trackIndex, 0, ImVec2(130, 20));
        ImGui::SameLine();
      }
    }
    ImGui::NewLine();

    // Session Grid
    for (int y = 0; y < session->scenes.size(); y++){
      for (int x = 0; x < session->tracks.size(); x++)
      {

        bool selected = session->isClipSelected(x, y);
        auto clip = session->selectClipByPosition(x, y);
        bool clipSelectable = false;
        // Trigger BUtton
        ImGui::PushID(x * session->scenes.size() + y);
        auto trigger = ImGui::Button("", ImVec2(20, 20));
        if(trigger){
          session->activatePosition(x, y);
        }
        ImGui::PopID();
        ImGui::SameLine();
        ImGui::PushID(y * session->scenes.size() + x  );
        if (clip.get() != nullptr)
        {
          auto counter = clip->playheadPosPercentage();
          switch (clip->getState())
          {
          case ClipState::PLAYING:
            ImGui::ProgressBar(counter, ImVec2(100, 20), clip->name.value.c_str());
            break;

          default:
            clipSelectable = ImGui::Selectable(clip->name.value.c_str(), session->isClipSelected(x, y), 0, ImVec2(100, 20));
            
            break;
          }
          
        }
        else
        {
          clipSelectable = ImGui::Selectable("---", session->isClipSelected(x, y), 0, ImVec2(100, 20));
        }
        ImGui::PopID();
        if (clipSelectable)
        {
          sessionPosition.first = x;
          sessionPosition.second = y;
          session->selectPosition(x, y);
        }
        
        ImGui::SameLine();
      }
      ImGui::NewLine();
    }
      
      bool sessionWindowOpen = true;
      ImGuiWindowFlags window_flags = 0;
      window_flags |= ImGuiWindowFlags_NoTitleBar;
      window_flags |= ImGuiWindowFlags_NoScrollbar;
      // window_flags |= ImGuiWindowFlags_MenuBar;
      window_flags |= ImGuiWindowFlags_NoMove;
      window_flags |= ImGuiWindowFlags_NoResize;
      window_flags |= ImGuiWindowFlags_NoCollapse;
      window_flags |= ImGuiWindowFlags_NoNav;
      window_flags |= ImGuiWindowFlags_NoNav;
      ImGui::PushItemWidth(100.f);
      ImGui::BeginChild("Session", ImVec2(ImGui::GetContentRegionAvail().x - 100.f, 250),ImGuiChildFlags_Border, window_flags);
      ImGui::Text("This is the session windows");
      ImGui::EndChild();

    }
  );
}

int main(int, char **)
{

  // Daw Setup Code
  MidiEventRegistry &mer = MidiEventRegistry::getInstance();
  auto midiEngine = MidiEngine(true);

  for (auto &[name, device] : midiEngine.inputDevices)
  {
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

  // namespace fs = std::filesystem;
  // auto HOME = (std::string)getenv("HOME");
  // fs::path fileDir = HOME + "/Documents";
  // std::vector<filesystem::directory_entry> dir;
  // ScrollView sv(dir);

  // for (auto &entry : fs::directory_iterator(fileDir)) {
  //   dir.emplace_back(entry);
  // }

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
  std::string input;
  ui(session, playHead);
  std::cout << "Exiting...\n";
}
