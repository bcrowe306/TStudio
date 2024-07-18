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

using namespace std::filesystem;
using namespace ImGui;

static  char searchText[64];



void BuildDirectory(filesystem::path rootPath){
  vector<path> subDirs;
  vector<path> subFiles;
  if(is_directory(rootPath)){
    // build temp vector of directories and files
    for(auto &entry: directory_iterator(rootPath)){
      if(entry.is_directory()){
        subDirs.push_back(entry.path());
      }
      else
      {
        if (choc::text::contains(choc::text::toLowerCase(entry.path().filename().string()), choc::text::toLowerCase(searchText)))
          subFiles.push_back(entry.path());
      }
    }
    // Build tree nodes
    // Only build tree there are files
    if (subFiles.size() > 0){
      if (TreeNode(rootPath.filename().c_str())) {
        for (auto &subDir : subDirs) {
          BuildDirectory(subDir);
        }
        for (auto &subFile : subFiles) {
          BuildDirectory(subFile);
        }
        TreePop();
      }
    } 
    else {
      if(subDirs.size() > 0){
        for (auto &subDir : subDirs) {
          BuildDirectory(subDir);
        }
      }
    }
      
  } else {
    Selectable(rootPath.filename().c_str());
  }
  subDirs.clear();
  subFiles.clear();
}



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
            
            InputText("Search", searchText, 64);
            BuildDirectory(session->browser->rootDirectory);
            
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