#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "imgui.h"
#include "core/Playhead.h"
#include "core/Session.h"
#include "ui/LayoutDimensions.h"
#include "ui/SessionCell.h"
#include "ui/TrackHeader.h"
#include <memory>

using namespace ImGui;

void MainView(shared_ptr<tstudio::Session> session, shared_ptr<tstudio::Playhead> playhead, ImVec2 position, ImVec2 size, float trackWidth)
{
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
  
  bool isFocused = ImGui::IsWindowFocused();
  // Implement Key commands
  if(isFocused){
    ImGuiKey start_key = ImGuiKey_NamedKey_BEGIN;
    for (ImGuiKey key = start_key; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)){
    }
  }
  // Tracks Header
  float trackHeight = 25.f;

  // Must get currentpos before we start tracks
  auto pos = ImGui::GetCursorScreenPos();
  auto rectMin = ImGui::GetItemRectMin();
  auto rectMax = ImGui::GetItemRectMax();
  for (int trackIndex = 0; trackIndex < session->tracks.size(); trackIndex++) {
    auto track = session->getTrackByIndex(trackIndex);
    if (track != nullptr) {
      bool selected = session->selectedTrackIndex() == trackIndex;
      auto trackHeaderWidget =
          TrackHeader(session, track, ImVec2(trackWidth, trackHeight), selected);
      if (trackHeaderWidget) {
        session->selectTrack(trackIndex);
      }
     
      ImGui::SameLine();
    }
  }
  // Draggable Code
  ImGuiIO &io = ImGui::GetIO();
  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  int oldIndex;
  int newIndex;
  // Shows the dragging line
  if (ImGui::IsMouseDragging(0) && IsMouseHit(rectMin, ImVec2(rectMax.x, rectMin.y + trackHeight), io.MousePos))
  {
    ImVec2 value_raw = ImGui::GetMouseDragDelta(0, 0.0f);
    oldIndex = (int)(io.MouseClickedPos->x - 250.f) / (int)trackWidth;
    newIndex = (int)(io.MousePos.x - 250.f) / (int)trackWidth;
    newIndex = (newIndex < session->tracks.size()) ? newIndex : (int)session->tracks.size() - 1;
    auto line_x = newIndex * trackWidth + pos.x;
    if(value_raw.x > 0){
      line_x += trackWidth;
    }
    draw_list->AddLine(ImVec2(line_x, pos.y), ImVec2(line_x, pos.y + trackHeight), U32FromHex(BOOL_ON_COLOR), 2.f);

    // Troubleshooting code
    // ImGui::Text("Track Header is dragged %d: %.1f : %.1f", index, value_raw.x, value_raw.y);
    // ImGui::Text("rectMin: %.1f,%.1f , rectMax: %.1f,%.1f", rectMin.x,rectMin.y, rectMax.x, rectMax.y);
  }
  if (ImGui::IsMouseReleased(0) && IsMouseHit(rectMin, ImVec2(rectMax.x, rectMin.y + trackHeight), io.MousePos))
  {
    oldIndex = (int)(io.MouseClickedPos->x - 250.f) / (int)trackWidth;
    newIndex = (int)(io.MousePos.x - 250.f) / (int)trackWidth;
    newIndex = (newIndex < session->tracks.size()) ? newIndex : (int)session->tracks.size() - 1;
    session->reorderTrack(oldIndex, newIndex);
  }
  ImGui::NewLine();
  ImGui::Separator();

  // Session Grid
  for (int y = 0; y < session->scenes.size(); y++) {
    for (int x = 0; x < session->tracks.size(); x++) {

      auto track = session->getTrackByIndex(x);
      auto clip = track->selectClip(y);
      SessionCell(std::pair<int, int>{x, y}, session, playhead, track, clip,
                  trackWidth);
      ImGui::SameLine();
    }
    ImGui::NewLine();
  }

  ImGui::End();
  ImGui::PopStyleVar(2);
  ImGui::PopStyleColor();
};

#endif // !MAINVIEW_H