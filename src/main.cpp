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
#include <charconv>
#include <cstddef>
#include <cstdlib>
#include <array>
#include <cstring>
#include <filesystem>
#include <hello_imgui/app_window_params.h>
#include <hello_imgui/hello_imgui_font.h>
#include <hello_imgui/imgui_theme.h>
#include <hello_imgui/runner_params.h>
#include <hello_imgui/screen_bounds.h>
#include "ui/FooterPanel.h"
#include "ui/MainView.h"
#include "ui/Sidebar.h"
#include "ui/TrackListItem.h"
#include "ui/TrackHeader.h"
#include "ui/SessionCell.h"
#include "ui/GridView.h"
#include "ui/ClipView.h"
#include "ui/Toolbar.h"
#include "ui/Colors.h"
#include "ui/LayoutDimensions.h"
#include <imgui.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "hello_imgui/hello_imgui.h"
#include "library/UUID_Gen.h"
#define CYAN "\x1B[36m"
#define WHITE "\x1B[37m"
#define OUT std::cout
#define END std::endl


using namespace tstudio;
using namespace placeholders;
using std::getenv;
typedef void (*fptr)();

void ui(shared_ptr<Session> session, shared_ptr<Playhead> playhead)
{
  HelloImGui::RunnerParams params;
  params.appWindowParams.windowGeometry.fullScreenMode = HelloImGui::FullScreenMode::FullMonitorWorkArea;
  params.fpsIdling.fpsIdle = 40.f;
  params.callbacks.SetupImGuiStyle = [&](){
    params.imGuiWindowParams.tweakedTheme.Theme = ImGuiTheme::ImGuiTheme_PhotoshopStyle;
    params.imGuiWindowParams.tweakedTheme.Tweaks.Rounding = 0.0f;
    params.imGuiWindowParams.tweakedTheme.Tweaks.RoundingScrollbarRatio = 0.0f;


  };
  params.callbacks.LoadAdditionalFonts = [](){
    HelloImGui::LoadFont("fonts/OpenSans.ttf", 16.f);
  };
  params.callbacks.ShowGui = [playhead, session]() 
  {
    // Get the window size
    auto space = ImGui::GetContentRegionAvail();
    // auto screenSize = runnerParams->appWindowParams.windowGeometry.size;
    float window_width = space.x;
    float window_height = space.y;
    // Set up panel sizes and positions
    float toolbar_height = 50.0f;
    float sidebar_width = 250.0f;
    float footer_height = 300.0f;
    float main_width = window_width - sidebar_width;
    float main_height = window_height - footer_height - toolbar_height;
    ImGui::GetStyle().WindowRounding = 0.0f;
    ImGui::GetStyle().WindowBorderSize = 0.1f;
    ImGui::GetStyle().FrameRounding = 0.0f;
    ImGui::GetStyle().GrabRounding = 0.0f;
    ImGui::GetStyle().PopupRounding = 0.0f;
    ImGui::GetStyle().ScrollbarRounding = 0.0f;


    Toolbar(session, playhead, ImVec2(0,0), ImVec2(window_width, toolbar_height));
    Sidebar(session, playhead, ImVec2(0, toolbar_height), ImVec2(sidebar_width, main_height));
    FooterPanel(session, playhead, ImVec2(0, toolbar_height + main_height), ImVec2(window_width, footer_height));
    MainView(session, playhead, ImVec2(sidebar_width, toolbar_height), ImVec2(main_width, main_height));
    
  };
  std::cout << "uiThread\n";
  return HelloImGui::Run(params);
  
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
    // std::thread uiThread(ui, session, playHead);
    std::cout << "Exiting...\n";
  }
