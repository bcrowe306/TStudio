#ifndef TOGGLEWIDGET_H
#define TOGGLEWIDGET_H

#include "ui/Colors.h"
#include <imgui.h>

bool ToggleWidget(const char *label, bool *v, const ImVec2 &size) {
  ImGuiIO &io = ImGui::GetIO();
  ImDrawList *draw_list = ImGui::GetWindowDrawList();

  ImGui::PushID(label); // Ensure unique ID for each widget
  ImGui::InvisibleButton(label, size);
  bool hovered = ImGui::IsItemHovered();
  bool clicked = ImGui::IsItemClicked();

  if (clicked) {
    *v = !*v; // Toggle state
  }

  ImVec2 p = ImGui::GetItemRectMin();
  ImVec2 p_max = ImGui::GetItemRectMax();

  // Set colors based on toggle state
  ImU32 color_on = ImGui::GetColorU32(ImVec4(0.89f, 0.76f, 0.07f, 1.0f));
  ImU32 color_off = ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
  ImU32 color = *v ? color_on : color_off;
  ImU32 text_color;
  *v ? text_color = color_off : text_color = ImGui::GetColorU32(ImVec4(0.8f, 0.8f, 0.8f, 1.0f));

  // Draw the rectangle
  draw_list->AddRectFilled(p, p_max, color,
                           4.0f); // 4.0f is the rounding radius

  // Draw text on top of the rectangle
  ImVec2 text_size = ImGui::CalcTextSize(label);
  ImVec2 text_pos = ImVec2(p.x + (size.x - text_size.x) * 0.5f,
                           p.y + (size.y - text_size.y) * 0.5f);
  draw_list->AddText(text_pos, text_color, label);

  ImGui::PopID();
  return clicked;
};
#endif // !TOGGLEWIDGET_H