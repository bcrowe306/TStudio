#ifndef COLORS_H
#define COLORS_H

#include <imgui.h>
#include <iostream>
#include <string>
#include <vector>

static ImU32 U32FromHex(const char * hex, float alpha = 1.0){
  int r256, g256, b256;
  std::sscanf(hex, "%02x%02x%02x", &r256, &g256, &b256);
  float r, g, b;
  r = (float)r256 / 255.f;
  g = (float)g256 / 255.f;
  b = (float)b256 / 255.f;
  return ImGui::GetColorU32(ImVec4(r, g, b, alpha));
}


const auto BLACK_COLOR ="000000";
const auto WHITE_COLOR ="FFFFFF";
const auto SESSION_BACKGROUND_COLOR ="979797";
const auto SESSION_CLIP_TRIGGER_COLOR = "D9D9D9";
const auto PLAY_COLOR = "90FF8D";
const auto RECORD_COLOR = "90FF8D";
const auto STOP_COLOR = "FF675D";
const auto STOPPING_COLOR = "E19A11";
const auto LAUNCHING_PLAY_COLOR = "E19A11";
const auto LAUNCHING_RECORDING_COLOR = "E1CC11";
const auto SESSION_BORDER_COLOR = "5C5C5C";
const auto BOOL_ON_COLOR = "FFC20C";
const auto TRACK_BACKGROUND_COLOR = "2E2F31";
const auto TRACK_HEADER_SELECTED_COLOR = "3E3F42";
const auto TEXT_LIGHT_COLOR = "CBCBCB";
const auto TEXT_DARK_COLOR = "211F1F";
const auto WINDOW_DARK_BACKGROUND_COLOR = "232323";

const std::vector<std::string> COLOR_MAP = {
    "E58F8F", "E5A48F", "E5AE8F", "EB9C63", "EBB563", "EBC563", "EBE663",
    "C8EB63", "AFEB63", "87E65A", "5AE668", "5AE6A3", "5AE6E6", "5ABCE6",
    "5BACF7", "85A0FE", "9A98FF", "B398FF", "D898FF", "F098FF", "FF98F4",
    "FF98BD", "FF98A4", 
};
// const ImU32 ButtonColor = ImGui::GetColorU32(ImVec4(0.113f, 0.113f, 0.113f, 1.0f));
// const ImU32 ToggleOn = ImGui::GetColorU32(ImVec4(0.89f, 0.76f, 0.07f, 1.0f));
// const ImU32 ToggleOff = ImGui::GetColorU32(ImVec4(0.113f, 0.113f, 0.113f, 1.0f));
// const ImU32 PlayButton = ImGui::GetColorU32(ImVec4(0.113f, 0.8, 0.113f, 1.0f));
// const ImU32 RecordButton = ImGui::GetColorU32(ImVec4(0.8, 0.113f, 0.113f, 1.0f));
// const ImU32 ButtonTextOn = ImGui::GetColorU32(ImVec4(0.8, 0.8, 0.8, 1.0f));
// const ImU32 ButtonTextOff = ImGui::GetColorU32(ImVec4(0.113f, 0.113f, 0.113f, 1.0f));
// const ImU32 ClipEvent = ImGui::GetColorU32(ImVec4(0.113f, 0.113f, 0.113f, 1.0f));

#endif // !COLORS_H