#ifndef CLIPDETAIL_H
#define CLIPDETAILL_H
#include "core/MidiClip.h"
#include "core/Playhead.h"
#include "core/Session.h"
#include <imgui.h>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "ui/ToggleWidget.h"

namespace tstudio {
static void ClipDetail(shared_ptr<Playhead> playhead, MidiClipType clip) {
  auto grayscale = 0.113f;
  auto grayColor = ImVec4(grayscale, grayscale, grayscale, 1.f);
  ImGui::PushStyleColor(ImGuiCol_ChildBg, grayColor);
  ImGui::BeginChild("Clip Details",
                    ImVec2(240.f, ImGui::GetContentRegionAvail().y));

  if (clip != MidiClipType(nullptr)) {
    ImGui::SetCursorPos(ImVec2(5.f, 10.f));
    int clipLength = clip->getLength();
    auto lengthPos = clip->songPosition;    
    auto barCount = lengthPos.bar;
    auto beatCount = lengthPos.beat;
    auto tickCount = lengthPos.tick;
    ImGui::Indent(20.f);
    ImGui::Text(clip->name.value.c_str());
    auto barTextInput = ImGui::DragInt("Bars", &barCount, 1, 2);
    auto beatTextInput = ImGui::DragInt("Beats", &beatCount, 1, 0, 3);
    auto tickTextInput = ImGui::DragInt("Tick", &tickCount, 1, 0, 480);
    if (barTextInput) {
      auto newLength = SongPosition::getTickCountFromSongPosition(
          barCount, beatCount, tickCount, playhead->tpqn, playhead->time_sig);
      clip->setLength(newLength);
    }
    if (beatTextInput) {
      auto newLength =SongPosition::getTickCountFromSongPosition(
          barCount, beatCount, tickCount, playhead->tpqn, playhead->time_sig);
      clip->setLength(newLength);
    }
    if (tickTextInput) {
      auto newLength = SongPosition::getTickCountFromSongPosition(
          barCount, beatCount, tickCount, playhead->tpqn, playhead->time_sig);
      clip->setLength(newLength);
    }
    auto isLooping = clip->getLoopStatus();
    bool loopToggleWidget = ToggleWidget("Loop", &isLooping, ImVec2(ImGui::GetContentRegionAvail().x, 20.f));
    if (loopToggleWidget) {
      clip->setLoopStatus(isLooping);
    }
  }
  ImGui::EndChild();
  ImGui::PopStyleColor();
}
}


#endif // !CLIPDETAIL_H
