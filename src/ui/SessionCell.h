#ifndef SESSIONCELL_H
#define SESSIONCELL_H

#include "core/MidiClip.h"
#include "core/Playhead.h"
#include "core/Session.h"
#include "core/TrackNode.h"
#include "ui/Colors.h"
#include <cstddef>
#include <imgui.h>
#include <memory>
#include <unordered_map>

using std::shared_ptr;
using namespace tstudio;

void DrawEmptyCell() {}

void DrawClipStates(ImDrawList *draw_list, shared_ptr<MidiClip> clip, shared_ptr<Playhead> playhead,
                    ImVec2 p_min, ImVec2 p_max,
                    float trigger_button_width = 30.f) {
  float margin = 1.0f;
  ImVec2 min = ImVec2(p_min.x + margin, p_min.y + margin);
  ImVec2 max = ImVec2(p_max.x - margin, p_max.y - margin);
  auto clipColor = U32FromHex(clip->color.value.c_str());
  auto clipBackgroundColor = U32FromHex(clip->color.value.c_str(), .4f);

  if (clip != MidiClipType(nullptr)) {
    switch (clip->getState()) {
    case tstudio::ClipState::LAUNCHING_PLAY:
      draw_list->AddRectFilled(min, ImVec2(min.x + trigger_button_width, max.y),
                               U32FromHex(LAUNCHING_PLAY_COLOR, .5f), 0.0f);
      draw_list->AddRectFilled(ImVec2(min.x + trigger_button_width, min.y), max,
                               clipColor, 0.0f);
      break;
    case tstudio::ClipState::LAUNCHING_RECORDING:
      draw_list->AddRectFilled(min, ImVec2(min.x + trigger_button_width, max.y),
                               U32FromHex(LAUNCHING_RECORDING_COLOR, .5f), 0.0f);
      draw_list->AddRectFilled(ImVec2(min.x + trigger_button_width, min.y), max,
                               clipColor, 0.0f);
      break;
    case tstudio::ClipState::PLAYING: {
      draw_list->AddRectFilled(min, ImVec2(min.x + trigger_button_width, max.y),
                               clipColor, 0.0f);

        // Change clip if is playing  
        if(playhead->getState() == PlayheadState::PLAYING || playhead->getState() == PlayheadState::RECORDING){
          auto progress = (float)clip->getCounter() / (float)clip->getLength();
          auto clip_start_x = (min.x + trigger_button_width);
          auto progress_width = progress * (max.x - clip_start_x);

          draw_list->AddRectFilled(ImVec2(min.x + trigger_button_width, min.y),
                                   max, clipBackgroundColor, 0.0f);
          draw_list->AddRectFilled(ImVec2(min.x + trigger_button_width, min.y),
                                   ImVec2(progress_width + clip_start_x, max.y),
                                   clipColor, 0.0f);
        }else{
          draw_list->AddRectFilled(ImVec2(min.x + trigger_button_width, min.y),
                                   max, clipColor, 0.0f);
        }
      break;
    }
      
    case tstudio::ClipState::RECORDING:
      draw_list->AddRectFilled(min, ImVec2(min.x + trigger_button_width, max.y),
                               U32FromHex(RECORD_COLOR, .5f), 0.0f);
      draw_list->AddRectFilled(ImVec2(min.x + trigger_button_width, min.y), max,
                               clipColor, 0.0f);
      break;

    case tstudio::ClipState::RECORDING_INITIAL: {
      draw_list->AddRectFilled(min, ImVec2(min.x + trigger_button_width, max.y),
                               U32FromHex(RECORD_COLOR), 0.0f);
      float time = ImGui::GetTime();
      bool blink = static_cast<int>(time / .25f) % 2 == 0;
      if (blink) {
        draw_list->AddRectFilled(ImVec2(min.x + trigger_button_width, min.y),
                                 max, clipColor, 0.0f);
      } else {
        draw_list->AddRectFilled(ImVec2(min.x + trigger_button_width, min.y),
                                 max, U32FromHex(BLACK_COLOR, 0.f), 0.0f);
      } break;
    }

    case tstudio::ClipState::STOPPING:
      draw_list->AddRectFilled(min, ImVec2(min.x + trigger_button_width, max.y),
                               U32FromHex(STOPPING_COLOR), 0.0f);
      draw_list->AddRectFilled(ImVec2(min.x + trigger_button_width, min.y), max,
                               clipColor, 0.0f);
      break;
    case tstudio::ClipState::STOPPED:
      draw_list->AddRectFilled(min, ImVec2(min.x + trigger_button_width, max.y),
                               clipColor, 0.0f);
      draw_list->AddRectFilled(ImVec2(min.x + trigger_button_width, min.y), max,
                               clipColor, 0.0f);
      break;
    default:
      break;
    }
  }
}

void SessionCell(std::pair<int, int> cellPosition, shared_ptr<Session> session,
                 shared_ptr<Playhead> playhead, shared_ptr<TrackNode> track,
                 shared_ptr<MidiClip> clip, float width = 135.f,
                 float height = 30.f) {
  float trigger_element_size = 10.f;
  float left_padding = 10.f;
  float top_padding = 10.f;
  float trigger_button_width = 30.f;
  bool selected = session->isClipSelected(cellPosition);
  float alpha = (selected) ? .2f : 0.f;

  ImVec2 cellSize{width, height};

  ImGuiIO &io = ImGui::GetIO();
  ImDrawList *draw_list = ImGui::GetWindowDrawList();

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
  auto cellColor = (selected) ? U32FromHex(BOOL_ON_COLOR, .2f)
                              : U32FromHex(BLACK_COLOR, .0f);

  // Create click functionality
  // Trigger click
  auto label = std::to_string(cellPosition.first + cellPosition.second * session->tracks.size());
  auto triggerButtonClicked = ImGui::InvisibleButton(
      (label + "trigger").c_str(), ImVec2(trigger_button_width, cellSize.y));

  if (triggerButtonClicked) {
    session->selectClipByPosition(cellPosition);
    session->activatePosition(cellPosition.first, cellPosition.second);
  }

  ImVec2 p = ImGui::GetItemRectMin();
  ImVec2 p_max = ImGui::GetItemRectMax();
  ImGui::SameLine();

  // Clip click
  bool cellClicked = ImGui::InvisibleButton(
      label.c_str(), ImVec2(width - trigger_button_width, height));
  if (cellClicked) {
    std::cout << label << std::endl;
    session->selectPosition(cellPosition);
  }
  ImVec2 p2 = ImGui::GetItemRectMin();
  ImVec2 p2_max = ImGui::GetItemRectMax();

  // Render cell

  if (clip != MidiClipType(nullptr)) {
    DrawClipStates(draw_list, clip, playhead, p, p2_max, trigger_button_width);
  } else {
    // Draw empty cell

    //   draw_list->AddRectFilled(p, p_max, cellColor, 0.0f);
    //   draw_list->AddRectFilled(p2, p2_max, cellColor, 0.0f);
  }

  const unordered_map<ClipState, ImU32> clipstate_color_map = {
      {ClipState::LAUNCHING_PLAY, U32FromHex(LAUNCHING_PLAY_COLOR)},
      {ClipState::LAUNCHING_RECORDING, U32FromHex(LAUNCHING_RECORDING_COLOR)},
      {ClipState::RECORDING_INITIAL, U32FromHex(LAUNCHING_RECORDING_COLOR)},
      {ClipState::RECORDING, U32FromHex(RECORD_COLOR)},
      {ClipState::PLAYING, U32FromHex(PLAY_COLOR)},
      {ClipState::STOPPING, U32FromHex(STOPPING_COLOR)},
      {ClipState::STOPPED, U32FromHex(SESSION_BORDER_COLOR)},
      {ClipState::DISABLED, U32FromHex(SESSION_BORDER_COLOR, 0.f)},
  };
  auto triggerIconColor = (clip != MidiClipType(nullptr))
                              ? clipstate_color_map.at(clip->getState())
                              : U32FromHex(SESSION_CLIP_TRIGGER_COLOR);
  if (clip == nullptr){
    if (track != nullptr) {
      if (track->arm.value) {
        // Armed: Draw circle element
        auto circle_center = ImVec2(
            p.x + left_padding + trigger_element_size / 2, p.y + height / 2);
        auto circle_radius = trigger_element_size / 2;
        draw_list->AddCircleFilled(circle_center, circle_radius,
                                   triggerIconColor, 0.f);

      } else {
        // Draw stop element
        auto start_y = p.y + (height / 2 - trigger_element_size / 2);
        auto trigger_start = ImVec2(p.x + left_padding, start_y);
        auto trigger_end = ImVec2(trigger_start.x + trigger_element_size,
                                  trigger_start.y + trigger_element_size);
        draw_list->AddRectFilled(trigger_start, trigger_end, triggerIconColor,
                                 0.f);
      }
    }
  }else{
    if (track != nullptr) {
        auto clipState = clip->getState();
        if (clipState == ClipState::PLAYING || clipState == ClipState::LAUNCHING_PLAY || clipState == ClipState::STOPPING) {
          // Draw play element
          auto start_y = p.y + (height / 2 - trigger_element_size / 2);
          auto trigger_start = ImVec2(p.x + left_padding, start_y);
          auto trigger_middle =
              ImVec2(trigger_start.x + trigger_element_size,
                     trigger_start.y + trigger_element_size / 2);
          auto trigger_end =
              ImVec2(trigger_start.x, trigger_start.y + trigger_element_size);
          draw_list->AddTriangleFilled(trigger_start, trigger_middle,
                                       trigger_end, triggerIconColor);
        } else if (clipState == ClipState::RECORDING || clipState == ClipState::RECORDING_INITIAL || clipState == ClipState::LAUNCHING_RECORDING) {
          // Armed: Draw circle element
          auto circle_center = ImVec2(
              p.x + left_padding + trigger_element_size / 2, p.y + height / 2);
          auto circle_radius = trigger_element_size / 2;
          draw_list->AddCircleFilled(circle_center, circle_radius,
                                     triggerIconColor, 0.f);
        }
        else{
          // Draw stop element
          auto start_y = p.y + (height / 2 - trigger_element_size / 2);
          auto trigger_start = ImVec2(p.x + left_padding, start_y);
          auto trigger_end = ImVec2(trigger_start.x + trigger_element_size,
                                    trigger_start.y + trigger_element_size);
          draw_list->AddRectFilled(trigger_start, trigger_end, triggerIconColor,
                                   0.f);
        }
    }
    // Draw Text
    // Draw text on top of the rectangle
    auto clipName = clip->name.value.c_str();
    ImVec2 text_size = ImGui::CalcTextSize(clipName);
    ImVec2 text_pos = ImVec2(p2.x + 5.f,
                             p.y + (height - text_size.y) * 0.5f);
    draw_list->AddText(text_pos, U32FromHex(TEXT_DARK_COLOR), clipName);
  }
  

  // Draw Border
  float border_thickness = 1.f;
  draw_list->AddLine(ImVec2(p.x, p_max.y), ImVec2(p2_max.x, p2_max.y),
                     U32FromHex(SESSION_BORDER_COLOR), border_thickness);
  draw_list->AddLine(ImVec2(p2_max.x, p.y), ImVec2(p2_max.x, p2_max.y),
                     U32FromHex(SESSION_BORDER_COLOR), border_thickness);
  ImGui::PopStyleVar();

  // Draw selected border
  if (selected) {
    draw_list->AddRect(p, p2_max, U32FromHex(BOOL_ON_COLOR), 0.f, NULL);
  }
}

#endif // !SESSIONCELL_H