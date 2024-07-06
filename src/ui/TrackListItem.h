#ifndef TRACKLISTITEM_H
#define TRACKLISTITEM_H

#include "core/Playhead.h"
#include "core/Session.h"
#include <imgui.h>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

bool TrackListItem(const char *id, char *text, ImVec2 size, ImVec4 baseColor,
                   bool &isSelected, bool &isEditing) {
  ImGui::PushID(id);

  // Convert base color to HSV
  float hue, saturation, value;
  ImGui::ColorConvertRGBtoHSV(baseColor.x, baseColor.y, baseColor.z, hue,
                              saturation, value);

  // Adjust brightness based on the state
  ImVec4 color = baseColor;
  ImVec4 newColor;
  float r,g,b;
  if (isEditing) {
    ImGui::ColorConvertHSVtoRGB(hue, saturation, value * 1.2f, r, g, b);
    color = ImVec4(r,g,b, baseColor.w);
  } else if (isSelected) {
    ImGui::ColorConvertHSVtoRGB(hue, saturation, value * 1.1f, r, g, b);
    color = ImVec4(r, g, b, baseColor.w);
  } else {
    color = baseColor;
  }


  ImGui::PushStyleColor(ImGuiCol_Header, color);
  ImGui::PushStyleColor(
      ImGuiCol_HeaderHovered,
      ImVec4(r,g,b,
             baseColor.w));
  ImGui::PushStyleColor(
      ImGuiCol_HeaderActive,
      ImVec4(r,g,b,
             baseColor.w));

  bool clicked = ImGui::Selectable("", isSelected,
                                   ImGuiSelectableFlags_AllowDoubleClick, size);
  if (clicked) {
    if (ImGui::IsMouseDoubleClicked(0)) {
      isEditing = true;
    } else {
      isSelected = true;
      isEditing = false;
    }
  }

  // Get the last item's rectangle
  ImVec2 rectMin = ImGui::GetItemRectMin();
  ImVec2 rectMax = ImGui::GetItemRectMax();

  if (isEditing) {
    ImGui::SetKeyboardFocusHere();
    ImGui::SetCursorScreenPos(ImVec2(
        rectMin.x + 5, rectMin.y + (size.y - ImGui::GetTextLineHeight()) /
                                       2)); // Center the text vertically

    ImGui::PushItemWidth(size.x -
                         10); // Adjust width of the input text to be the same
                              // size as the rectangle with some padding
    ImGui::PushStyleColor(ImGuiCol_FrameBg,
                          ImVec4(0, 0, 0, 0)); // Transparent background
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1)); // White text
    ImGui::PushStyleColor(ImGuiCol_Border,
                          ImVec4(0, 0, 0, 0)); // Transparent border

    if (ImGui::InputText("##edit", text, 256,
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
      isEditing = false; // Exit editing mode when Enter is pressed
    }

    ImGui::PopStyleColor(3);
    ImGui::PopItemWidth();
  } else {
    ImGui::SetCursorScreenPos(ImVec2(
        rectMin.x + 5, rectMin.y + (size.y - ImGui::GetTextLineHeight()) /
                                       2)); // Center the text vertically
    ImGui::Text("%s", text);
  }

  ImGui::PopStyleColor(3);
  ImGui::PopID();

  return clicked;
}

#endif // !TRACKLISTITEM_H
