#ifndef SIDEBAR_H
#define SIDEBAR_H

#include "core/Playhead.h"
#include "core/Session.h"
#include "imgui.h"
#include "library/Browser.h"
#include "text/choc_StringUtilities.h"
#include "ui/Colors.h"
#include <memory>
#include <string>
#include <filesystem>
#include <vector>
#include "library/Browser.h"
#include <future>

using namespace std::filesystem;
using namespace ImGui;
using std::async;
using std::future;

static  char searchText[64];

void BuildDirectory(BrowserNode &browserNode){
  if(is_directory(browserNode.filePath)){
    // Only build tree there are files

      if (TreeNode(browserNode.fileName.c_str()))
      {
        for (auto &node : browserNode.children){
            BuildDirectory(node);

        }
        TreePop();
      }
  } else {
    Selectable(browserNode.fileName.c_str());
  }
}


int selectedBrowserNodeIndex = 0;
future<void> filterFuture;
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
            
            auto searchTextInput = InputText("Search", searchText, 64);
            auto &browserNode = session->browser->browserItems[selectedBrowserNodeIndex];
            
            if(searchTextInput){
              auto searchTextString = string(choc::text::trimEnd(searchText));
              if(searchTextString.size() > 3){
                filterFuture = async(launch::async, &BrowserNode::setFilter, &browserNode, searchTextString);
              }else{
                filterFuture = async(launch::async, &BrowserNode::setFilter, &browserNode, "");
              }
              // browserNode.setFilter(std::string(searchText));
              
            }
            BuildDirectory(browserNode);

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