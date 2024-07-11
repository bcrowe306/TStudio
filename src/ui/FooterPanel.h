#ifndef FOOTERPANEL_H
#define FOOTERPANEL_H

#include "core/Playhead.h"
#include "core/Session.h"
#include "imgui.h"
#include "ui/ClipView.h"
#include <memory>

void FooterPanel(shared_ptr<tstudio::Session> session, shared_ptr<tstudio::Playhead> playhead, ImVec2 position, ImVec2 size){
  // Footer Panel
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
  ImGui::SetNextWindowPos(position);
  ImGui::SetNextWindowSize(size);
  ImGui::Begin("Footer", NULL,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration);
  // Tabs in the footer panel
  if (ImGui::BeginTabBar("FooterTabs")) {
    if (ImGui::BeginTabItem("Device")) {
      ImGui::Text("This is the Device tab.");
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Clip", NULL)) {
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
}

#endif // !FOOTERPANEL_H