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
#include "ui/ClipView.h"
#include "libusb.h"
#include <__functional/bind_front.h>
#include <cstddef>
#include <cstdlib>
#include <array>
#include <filesystem>
#include <hello_imgui/app_window_params.h>
#include <hello_imgui/imgui_theme.h>
#include <hello_imgui/runner_params.h>
#include <hello_imgui/screen_bounds.h>
#include <imgui.h>
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
typedef void (*fptr)();

void ui(shared_ptr<Session> session, shared_ptr<Playhead> playhead)
{
  HelloImGui::RunnerParams params;
  params.appWindowParams.windowGeometry.fullScreenMode = HelloImGui::FullScreenMode::FullMonitorWorkArea;
  params.callbacks.SetupImGuiStyle = [&](){
    params.imGuiWindowParams.tweakedTheme.Theme = ImGuiTheme::ImGuiTheme_PhotoshopStyle;
    params.imGuiWindowParams.tweakedTheme.Tweaks.Rounding = 0.0f;
    params.imGuiWindowParams.tweakedTheme.Tweaks.RoundingScrollbarRatio = 0.0f;


  };
  params.callbacks.ShowGui = [playhead, session]() 
  {
    // Get the window size
    int window_width, window_height;
    auto space = ImGui::GetContentRegionAvail();
    // auto screenSize = runnerParams->appWindowParams.windowGeometry.size;
    window_width = space.x;
    window_height = space.y;
    // Set up panel sizes and positions
    float toolbar_height = 50.0f;
    float sidebar_width = 250.0f;
    float footer_height = 250.0f;
    float main_width = window_width - sidebar_width;
    float main_height = window_height - footer_height - toolbar_height;
    ImGui::GetStyle().WindowRounding = 0.1f;
    ImGui::GetStyle().WindowBorderSize = 0.1f;
    ImGui::GetStyle().FrameRounding = 0.0f;
    ImGui::GetStyle().GrabRounding = 0.0f;
    ImGui::GetStyle().PopupRounding = 0.0f;
    ImGui::GetStyle().ScrollbarRounding = 0.0f;


    // Toolbar Panel
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(window_width, toolbar_height));
    ImGui::Begin("Toolbar", NULL,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoDecoration);
    static std::pair<int, int> sessionPosition;
    auto tempo = playhead->getTempo();
    ImGui::AlignTextToFramePadding();
    ImGui::PushItemWidth(100.f);
    auto tempoText = ImGui::InputScalar("Tempo", ImGuiDataType_Float, &tempo,
                                        true ? &f32_one : NULL);
    if (tempoText) {
      if (tempo > 30 && tempo < 300) {
        playhead->setTempo(tempo);
      }
    }
    ImGui::SameLine();

    std::string buttonLabel;
    float playButtonHue = 0.35f;
    switch (playhead->getState()) {
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
    // ImGui::PushStyleColor(ImGuiCol_Button,
    // (ImVec4)ImColor::HSV(playButtonHue, 0.6f, 0.6f));
    // ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
    // (ImVec4)ImColor::HSV(playButtonHue / 7.0f, 0.7f, 0.7f));
    // ImGui::PushStyleColor(ImGuiCol_ButtonActive,
    // (ImVec4)ImColor::HSV(playButtonHue / 7.0f, 0.8f, 0.8f));
    auto playButton = ImGui::Button(playhead->isPlaying() ? "Stop" : "Play");
    ImGui::SameLine();
    if (playButton) {
      playhead->togglePlay();
    }
    // ImGui::PopID();
    ImGui::SameLine();

    // RecordButton
    float buttonHue = playhead->isRecording() ? 0.0f : 0.5f;
    ImGui::PushID("recordButton");
    ImGui::PushStyleColor(ImGuiCol_Button,
                          (ImVec4)ImColor::HSV(buttonHue, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          (ImVec4)ImColor::HSV(buttonHue / 7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          (ImVec4)ImColor::HSV(buttonHue / 7.0f, 0.8f, 0.8f));

    auto recordButton = ImGui::Button(buttonLabel.c_str());
    if (recordButton) {
      playhead->toggleRecord();
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();

    ImGui::SameLine();
    ImGui::PushID("stopButton");
    auto stopButton = ImGui::Button("Stop");
    if (stopButton) {
      playhead->stop();
    }
    ImGui::PopID();
    ImGui::SameLine();

    // addTrackButton
    ImGui::PushID("addTrackButton");
    auto addTrackButton = ImGui::Button("Add Track");
    if (addTrackButton) {
      session->addTrack();
    }
    ImGui::PopID();
    ImGui::SameLine();

    // addSceneButton
    ImGui::PushID("addSceneButton");
    auto addSceneButton = ImGui::Button("Add Scene");
    if (addSceneButton) {
      session->addScene();
    }
    ImGui::PopID();

    ImGui::SameLine();
    // Metronome
    static bool check = playhead->getMetronomeEnabled();
    auto metronomeCheckbox = ImGui::Checkbox("Metronome", &check);
    ImGui::SameLine();
    if (metronomeCheckbox) {
      playhead->toggleMetronomeEnabled();
    }

    // positionText
    ImGui::PushID("positionText");
    auto positionString = (std::to_string(sessionPosition.first) + " : " +
                           std::to_string(sessionPosition.second))
                              .c_str();
    ImGui::Text(positionString);
    ImGui::PopID();
    ImGui::End();

    // Sidebar Panel
    ImGui::SetNextWindowPos(ImVec2(0, toolbar_height));
    ImGui::SetNextWindowSize(ImVec2(sidebar_width, main_height));
    ImGui::Begin("Browser", NULL,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse);
    ImGui::Text("This is the sidebar.");
    ImGui::End();

    // Footer Panel
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    ImGui::SetNextWindowPos(ImVec2(0, toolbar_height + main_height));
    ImGui::SetNextWindowSize(ImVec2(window_width, footer_height));
    ImGui::Begin("Footer", NULL,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoDecoration);
    // Tabs in the footer panel
    if (ImGui::BeginTabBar("FooterTabs")) {
      if (ImGui::BeginTabItem("Device")) {
        ImGui::Text("This is the Device tab.");
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Clip",NULL, ImGuiTabItemFlags_SetSelected)) {
        ClipView(session, playhead);
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Track")) {
        ImGui::Text("This is the Track tab.");
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
    ImGui::End();
    ImGui::PopStyleVar(1);

    // Main Panel
    ImGui::SetNextWindowPos(ImVec2(sidebar_width, toolbar_height));
    ImGui::SetNextWindowSize(ImVec2(main_width, main_height));
    ImGui::Begin("Main", NULL,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    // Tracks Header
    for (int trackIndex = 0; trackIndex < session->tracks.size();
         trackIndex++) {
      auto track = session->getTrackByIndex(trackIndex);
      if (track != nullptr) {
        ImGui::Selectable(track->name.value.c_str(),
                          session->selectedTrackIndex() == trackIndex, 0,
                          ImVec2(130, 20));
        ImGui::SameLine();
      }
    }
    ImGui::NewLine();
    ImGui::Separator();

    // Session Grid
    for (int y = 0; y < session->scenes.size(); y++) {
      for (int x = 0; x < session->tracks.size(); x++) {

        bool selected = session->isClipSelected(x, y);
        auto clip = session->selectClipByPosition(x, y);
        bool clipSelectable = false;
        // Trigger BUtton
        ImGui::PushID(x * session->scenes.size() + y);
        auto trigger = ImGui::Button("", ImVec2(20, 20));
        if (trigger) {
          session->activatePosition(x, y);
        }
        ImGui::PopID();
        ImGui::SameLine();
        ImGui::PushID(y * session->scenes.size() + x);
        if (clip.get() != nullptr) {
          auto counter = clip->playheadPosPercentage();
          switch (clip->getState()) {
          case ClipState::PLAYING:
            ImGui::ProgressBar(counter, ImVec2(100, 20),
                               clip->name.value.c_str());
            break;

          default:
            clipSelectable = ImGui::Selectable(clip->name.value.c_str(),
                                               session->isClipSelected(x, y), 0,
                                               ImVec2(100, 20));

            break;
          }

        } else {
          clipSelectable = ImGui::Selectable(
              "---", session->isClipSelected(x, y), 0, ImVec2(100, 20));
        }
        ImGui::PopID();
        if (clipSelectable) {
          sessionPosition.first = x;
          sessionPosition.second = y;
          session->selectPosition(x, y);
        }

        ImGui::SameLine();
      }
      ImGui::NewLine();
    }
    ImGui::End();
  };
  HelloImGui::Run(params);
};

int main(int, char **)
{

    // Daw Setup Code
    MidiEventRegistry &mer = MidiEventRegistry::getInstance();
    auto midiEngine = MidiEngine(true);

    for (auto &[name, device] : midiEngine.inputDevices) {
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
    mer.subscribe(filter, [&](MidiMsg &event) {
      char knob1 = 70;
      char knob2 = 71;
      char knob3 = 72;
      char knob4 = 73;
      char knob5 = 74;
      char knob6 = 75;
      char knob7 = 76;
      char knob8 = 77;

      if (event.getNoteNumber().note == knob1 && event.getVelocity() == 1) {
        session->nextTrack();
      }
      if (event.getNoteNumber().note == knob1 && event.getVelocity() == 127) {
        session->prevTrack();
      }
      if (event.getNoteNumber().note == knob2 && event.getVelocity() == 1) {
        session->nextScene();
      }
      if (event.getNoteNumber().note == knob2 && event.getVelocity() == 127) {
        session->prevScene();
      }

      if (event.getNoteNumber().note == knob3 && event.getVelocity() == 1) {
        playHead->setTempo(playHead->getTempo() + 1.0f);
      }
      if (event.getNoteNumber().note == knob3 && event.getVelocity() == 127) {
        playHead->setTempo(playHead->getTempo() - 1.0f);
      }
    });
    std::string input;
    ui(session, playHead);
    std::cout << "Exiting...\n";
  }
