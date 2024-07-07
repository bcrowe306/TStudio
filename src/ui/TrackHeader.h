#ifndef TRACKHEADER_H
#define TRACKHEADER_H

#include <imgui.h>
#include "Colors.h"
#include "core/TrackNode.h"
#include "ui/Colors.h"
// TODO: Implement Track Header
// TODO: Implement Clip Stop Button
// TODO: Re-arrange track order by drag
// TODO: Rename track on double click, or right click context menu
// TODO: Track Color Select on right click, context menu
bool TrackHeader(
    shared_ptr<tstudio::TrackNode> track, const ImVec2 &size, bool &selected,
    ImU32 activeColor = ImGui::GetColorU32(ImVec4(0.89f, 0.76f, 0.07f, 1.0f)),
    ImU32 defaultColor = ImGui::GetColorU32(ImVec4(0.113f, 0.113f, 0.113f,
                                                   1.0f))) {
  ImGuiIO &io = ImGui::GetIO();
  ImDrawList *draw_list = ImGui::GetWindowDrawList();
    auto label = track->name.value.c_str();
  ImGui::PushID(label); // Ensure unique ID for each widget
  ImGui::InvisibleButton(label, size);
  bool hovered = ImGui::IsItemHovered();
  bool clicked = ImGui::IsItemClicked();

  ImVec2 p = ImGui::GetItemRectMin();
  ImVec2 p_max = ImGui::GetItemRectMax();

  auto trackColor =  U32FromHex(track->color.value.c_str());

  ImU32 text_color =
      selected ? U32FromHex(track->color.value.c_str()) : U32FromHex(TEXT_LIGHT_COLOR);
    ImU32 trackHeaderBackgroundColor = (selected) ? U32FromHex(TRACK_HEADER_SELECTED_COLOR) : U32FromHex(TRACK_BACKGROUND_COLOR);

  // Draw the rectangle header
    draw_list->AddRectFilled(p, p_max, trackHeaderBackgroundColor,
                             0.0f); // 4.0f is the rounding radius
    float left_padding = 10.f;
    float trigger_element_size = 10.f;
    // Draw stop element
    auto start_y = p.y + (size.y / 2 - trigger_element_size / 2);
    auto trigger_start = ImVec2(p.x + left_padding, start_y);
    auto trigger_end = ImVec2(trigger_start.x + trigger_element_size,
                              trigger_start.y + trigger_element_size);
    draw_list->AddRectFilled(trigger_start, trigger_end, trackColor, 0.f);

    // Draw text on top of the rectangle
    ImVec2 text_size = ImGui::CalcTextSize(label);
    ImVec2 text_pos = ImVec2(p.x + (size.x - text_size.x) * 0.5f,
                             p.y + (size.y - text_size.y) * 0.5f);
    draw_list->AddText(text_pos, text_color, label);

    ImGui::PopID();
    return clicked;
};
#endif // !TRACKHEADER_H