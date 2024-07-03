#ifndef CLIPVIEW_H
#define CLIPVIEW_H
#include <imgui.h>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "core/MidiClip.h"
#include "core/Playhead.h"
#include "core/Session.h"
#include "core/ClipEvent.h"

using namespace tstudio;

struct GridEvent {
  ImVec2 position;
  ImVec2 size;
  ImU32 color;
  bool isClicked;
};

struct mEvent {
    int start;
    int duration;
    bool isClicked;
};


static unordered_map<std::string, int> divisionMap ={
    {"1/2", 960},
    {"1/4", 480},
    {"1/8", 240},
    {"1/16", 120},
    {"1/32", 60},
};

void DrawGrid(ImDrawList *draw_list, ImVec2 origin, ImVec2 size, int clip_length, std::string divisionString, int y_range) {
  ImU32 color = IM_COL32(80, 80, 80, 127);
  int division = divisionMap[divisionString];
  auto x_spacing = (float)division / (float)clip_length * size.x;
  auto y_spacing = size.y / (float)y_range;
  for (float x = origin.x; x < origin.x + size.x; x += x_spacing) {
    draw_list->AddLine(ImVec2(x, origin.y), ImVec2(x, origin.y + size.y),
                       color);
  }
  for (float y = origin.y; y < origin.y + size.y; y += y_spacing) {
    draw_list->AddLine(ImVec2(origin.x, y), ImVec2(origin.x + size.x, y),
                       color);
  }
}

void DrawGridEvents(ImDrawList* draw_list, std::vector<GridEvent>& events)
{
    ImGuiIO& io = ImGui::GetIO();
    for (const auto& event : events)
    {
        draw_list->AddRectFilled(event.position, ImVec2(event.position.x + event.size.x, event.position.y + event.size.y), event.color);

        // Check for click within the event box
        if (ImGui::IsMouseHoveringRect(event.position, ImVec2(event.position.x + event.size.x, event.position.y + event.size.y)) && ImGui::IsMouseClicked(0))
        {
            // Handle event click (here we are just setting isClicked to true for demonstration)
            const_cast<GridEvent&>(event).isClicked = true; // Note: This cast is necessary to modify the const event in the vector
        }
    }
}
void DrawMidiEvents(ImDrawList* draw_list,ImVec2 &origin, ImVec2 &windowSize, shared_ptr<MidiClip> clip)
{
    ImGuiIO& io = ImGui::GetIO();
    auto length = clip->getLength();
    auto minMaxNotes = clip->getNoteRange();
    auto noterange = minMaxNotes.second - minMaxNotes.first;
    float y_spacing;
    (noterange == 0) ? y_spacing = windowSize.y : y_spacing= windowSize.y / (float)noterange;

    for (const auto& [noteNumber, events] : clip->data)
    {
        for(auto &event : events){
          auto xStart = origin.x + 1 + ((float)event.start / (float)length) * windowSize.x;
          auto yStart = (origin.y + windowSize.y - 2) - y_spacing * (float)(noteNumber - minMaxNotes.first);
          auto yEnd = yStart - y_spacing + 3;
          auto xEnd =
              origin.x - 1 + 
              ((event.start + event.getDuration()) / (float)length) * windowSize.x;
          auto startPosition = ImVec2(xStart, yStart);
          auto endPosition = ImVec2(xEnd, yEnd);
          draw_list->AddRectFilled(startPosition, endPosition,
                                   IM_COL32(255, 0, 0, 255));

          // Check for click within the event box
          if (ImGui::IsMouseHoveringRect(startPosition, endPosition) &&
              ImGui::IsMouseClicked(0)) {
            // Handle event click (here we are just setting isClicked to true
            // for demonstration) const_cast<cli&>(event).isClicked = true; //
            // Note: This cast is necessary to modify the const event in the
            // vector
          }
        }
        
    }
}

void DrawPlayhead(ImDrawList* draw_list,ImVec2 &origin, ImVec2 &windowSize, shared_ptr<MidiClip> clip){
  ImU32 color = IM_COL32(225, 225, 225, 127);
  auto clipPosition = windowSize.x * ((float)clip->getCounter() / (float)clip->getLength()) + origin.x;
  draw_list->AddLine(ImVec2(clipPosition, origin.y), ImVec2(clipPosition, origin.y + windowSize.y), color, 5.f);
}

    static void ClipView(shared_ptr<Session> session,
                         shared_ptr<Playhead> playhead) {
  auto clip = session->selectedClip();
  // Details
  static float grayscale = 0.113f;
  auto grayColor = ImVec4(grayscale, grayscale, grayscale, 1.f);
  static float x_gridSize = 20.f;
  static float y_gridSize = 20.f;
  ImGui::PushStyleColor(ImGuiCol_ChildBg, grayColor);
  ImGui::BeginChild("Clip Details",
                    ImVec2(240.f, ImGui::GetContentRegionAvail().y));
  ImGui::SliderFloat("x-grid", &x_gridSize, 2.f, 200.f);
  ImGui::SliderFloat("y-grid", &y_gridSize, 2.f, 200.f);

  if (clip != MidiClipType(nullptr)) {
    int clipLength = clip->getLength();
    ImGui::Text(clip->name.value.c_str());
    auto isLengthChanged = ImGui::DragInt("Length", &clipLength, 2, 2);
    if (isLengthChanged) {
      clip->setLength(clipLength);
    }
  }
  ImGui::EndChild();
  ImGui::PopStyleColor();
  ImGui::SameLine();
  // Clip Event
  ImGui::PushStyleColor(ImGuiCol_ChildBg, grayColor);
  ImGui::BeginChild("Clip Events", ImVec2(ImGui::GetContentRegionAvail().x,
                                          ImGui::GetContentRegionAvail().y));
  ImDrawList *draw_list = ImGui::GetWindowDrawList();

  // Draw a scalable grid
  if (clip != MidiClipType(nullptr)) {
    ImVec2 origin = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();
    ImU32 grid_color = IM_COL32(80, 80, 80, 127);
    auto minMaxNotes = clip->getNoteRange();
    auto noterange = minMaxNotes.second - minMaxNotes.first;
    DrawGrid(draw_list, origin, size, clip->getLength(), "1/16", noterange);
    ImGui::Text(clip->name.value.c_str());
    // Draw grid events
    DrawMidiEvents(draw_list, origin, size, clip);
    DrawPlayhead(draw_list, origin, size, clip);
    // Check and display event clicks
    // for (const auto &event : gridEvents) {
    //   if (event.isClicked) {
    //     ImGui::Text("Event at (%.0f, %.0f) was clicked!",
    //                 event.position.x, event.position.y);
    //   }
    // }
  }

  ImGui::EndChild();
  ImGui::PopStyleColor();
}

#endif // !CLIPVIEW_H