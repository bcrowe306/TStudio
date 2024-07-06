#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "imgui.h"
#include "core/Playhead.h"
#include "core/Session.h"
#include "ui/LayoutDimensions.h"
#include "ui/SessionCell.h"
#include "ui/TrackHeader.h"
#include <memory>


void MainView(shared_ptr<tstudio::Session> session, shared_ptr<tstudio::Playhead> playhead, ImVec2 position, ImVec2 size) {
  // Main Panel
  
  ImGui::SetNextWindowPos(position);
  ImGui::SetNextWindowSize(size);
  ImGui::PushStyleColor(ImGuiCol_WindowBg,
                        U32FromHex(SESSION_BACKGROUND_COLOR));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
  ImGui::Begin("Main", NULL,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
  ImGuiIO &io = ImGui::GetIO();
  bool isFocused = ImGui::IsWindowFocused();
  // Implement Key commands
  if(isFocused){
    ImGuiKey start_key = ImGuiKey_NamedKey_BEGIN;
    for (ImGuiKey key = start_key; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)){
    }
  }
  // Tracks Header
  float width = 145.f;
  for (int trackIndex = 0; trackIndex < session->tracks.size(); trackIndex++) {
    auto track = session->getTrackByIndex(trackIndex);
    if (track != nullptr) {
      bool selected = session->selectedTrackIndex() == trackIndex;
      auto trackHeaderWidget =
          TrackHeader(track, ImVec2(width, 25.f), selected);
      if (trackHeaderWidget) {
        session->selectTrack(trackIndex);
      }
     
      ImGui::SameLine();
    }
  }
  ImGui::NewLine();
  ImGui::Separator();

  // Session Grid
  for (int y = 0; y < session->scenes.size(); y++) {
    for (int x = 0; x < session->tracks.size(); x++) {

      auto clip = session->selectClipByPosition(x, y);
      auto track = session->getTrackByIndex(x);
      SessionCell(std::pair<int, int>{x, y}, session, playhead, track, clip,
                  width);
      ImGui::SameLine();
    }
    ImGui::NewLine();
  }
  ImGui::End();
  ImGui::PopStyleVar(2);
  ImGui::PopStyleColor();
};


#endif // !MAINVIEW_H