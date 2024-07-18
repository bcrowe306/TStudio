#ifndef FOOTERPANEL_H
#define FOOTERPANEL_H

#include "core/Playhead.h"
#include "core/Session.h"
#include "imgui.h"
#include "ui/ClipView.h"
#include <cstddef>
#include <cstdio>
#include <memory>

static int selectedFooterTab = 2;

void FooterPanel(shared_ptr<tstudio::Session> session, shared_ptr<tstudio::Playhead> playhead, ImVec2 position, ImVec2 size){
  // Footer Panel
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
  ImGui::SetNextWindowPos(position);
  ImGui::SetNextWindowSize(size);
  ImGui::Begin("Footer", NULL,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration);
  // Tabs in the footer panel
  auto deviceTabSelected = (selectedFooterTab == 0) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
  auto clipTabSelected = (selectedFooterTab == 1) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
  auto trackTabSelected = (selectedFooterTab == 2) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
  auto isOpen = true;
  if (ImGui::BeginTabBar("FooterTabs" )) {
    if (ImGui::BeginTabItem("DeviceTab",&isOpen)) {
      selectedFooterTab = 0;
      ImGui::Text("This is the Device tab.");
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("ClipTab", &isOpen)) {
      selectedFooterTab = 1;
      ClipView(session, playhead);
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("TrackTab", &isOpen)) {
      selectedFooterTab = 2;
      ImGui::Text("This is the Track tab.");
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
  ImGui::End();
  ImGui::PopStyleVar(1);
}

#endif // !FOOTERPANEL_H