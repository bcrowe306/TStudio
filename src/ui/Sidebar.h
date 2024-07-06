#ifndef SIDEBAR_H
#define SIDEBAR_H

#include "core/Playhead.h"
#include "core/Session.h"
#include "imgui.h"
#include "library/Browser.h"
#include "ui/Colors.h"
#include <memory>
#include <string>

void Sidebar(std::shared_ptr<tstudio::Session> session, shared_ptr<tstudio::Playhead> playhead, ImVec2 position, ImVec2 size) {
  // Sidebar Panel
  ImGui::SetNextWindowPos(position);
  ImGui::SetNextWindowSize(size);
  ImGui::Begin("Browser", NULL,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration);

  if (ImGui::BeginTabBar("FooterTabs")) {
    if (ImGui::BeginTabItem("Browser")) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, U32FromHex(WINDOW_DARK_BACKGROUND_COLOR));
        ImGui::BeginChild("Browser");
            if(ImGui::TreeNode("Documents")){
                ImGui::Text(std::to_string(session->browser->dir.size()).c_str());
              for (auto &entry : session->browser->dir) {
                if(ImGui::TreeNode(entry.path().filename().string().c_str())){

                ImGui::TreePop();
                }
              }
            ImGui::TreePop();
            }
            
            ImGui::Text("This is the Device tab.");
        ImGui::EndChild();
        ImGui::PopStyleColor();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Devices", NULL)) {
      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }
  ImGui::End();
  
  }

#endif // !SIDEBAR_H