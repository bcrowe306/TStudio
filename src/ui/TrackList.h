#ifndef TRACKLIST_H
#define TRACKLIST_H

#include "core/Playhead.h"
#include "core/Session.h"
#include "imgui.h"
#include "ui/Colors.h"
#include "ui/LayoutDimensions.h"
#include "ui/SessionCell.h"
#include "ui/TrackHeader.h"
#include "ui/LetterButton.h"
#include "ui/StereoMeter.h"
#include "ui/Utility.h"
#include "ui/imgui-knobs.h"
#include <memory>
#include <string>

using namespace tstudio;
using namespace ImGui;

using std::shared_ptr;


void TrackList(shared_ptr<Session> session, shared_ptr<Playhead> playHead, ImVec2 position, ImVec2 size, float trackWidth){
  // Testing TrackList
  float trackListHeight = size.y;
  float trackListWidth = size.x;
  float topSectionHeight = 25.f;
  float bottomSectionHeight = 20.f;
  float meterWidth = 12.f;
  float sliderWidth = 15.f;
  float trackButtonSize = 25.f;
  float padding_x = 10.f;
  float padding_y = 15.f;

  float middleSectionStart_y = position.y + topSectionHeight;
  float bottomSectionStart_y = position.y + size.y - bottomSectionHeight; 
  float middleSectionSize_y = bottomSectionStart_y - middleSectionStart_y;
  float bottomSectionEnd_y = position.y + size.y; 

  float meterStart_y = middleSectionStart_y + padding_y;
  float meterEnd_y = bottomSectionStart_y - padding_y;
  float meterSize_y = meterEnd_y - meterStart_y;

  float innerItemsWidth =(padding_x * 3) + trackButtonSize + meterWidth + sliderWidth;
  float panElementHeight = 80.f;
  float innerButtonsHeight = (padding_y * 2) + (trackButtonSize * 3) + panElementHeight;
  float startVCenterButtons_y = middleSectionStart_y + ((middleSectionSize_y - innerButtonsHeight) / 2);

  auto draw_list = GetWindowDrawList();
  ImGuiIO &io = ImGui::GetIO();
  SetNextWindowPos(position);
  PushStyleVar(ImGuiStyleVar_ChildRounding, 0.f);
  PushStyleColor(ImGuiCol_ChildBg, U32FromHex(SESSION_BACKGROUND_COLOR));
  BeginChild("tracklist", size, ImGuiChildFlags_Border);
    for (int i = 0; i < session->tracks.size(); i++)
    {
        auto track = session->tracks[i];
        auto start_x = position.x + (trackWidth * i);
        auto end_x = position.x + (trackWidth * i) + trackWidth;
        float startCenter_x = start_x + ( (trackWidth - innerItemsWidth) / 2);

        auto bottomSectionMin = ImVec2(start_x, bottomSectionStart_y);
        auto bottomSectionMax = ImVec2(end_x, bottomSectionEnd_y);

        // Determine track selected
        bool selected = session->selectedTrackIndex() == i;

        // Determine track colors
        ImU32 trackFooterBgColor = (selected) ? U32FromHex(TRACK_HEADER_SELECTED_COLOR) : U32FromHex(TRACK_BACKGROUND_COLOR);
        ImU32 trackFooterTextColor = (selected) ? U32FromHex(track->color.value.c_str()) : U32FromHex(TEXT_LIGHT_COLOR);
        ImU32 trackColor = (selected) ? U32FromHex(SESSION_BACKGROUND_COLOR) : U32FromHex(TRACK_LIST_BACKGROUND_COLOR);

        // Draw selected track if necessary
        if (selected)
        {
          draw_list->AddRectFilled(ImVec2(start_x, position.y), ImVec2(end_x, position.y + size.y), trackColor, 0.f);
        }else{
          draw_list->AddRectFilled(ImVec2(start_x, position.y), ImVec2(end_x, position.y + size.y), trackColor, 0.f);
        }


        // Draw Top Section Border
        draw_list->AddLine(ImVec2(start_x, middleSectionStart_y), ImVec2(end_x, middleSectionStart_y), U32FromHex(SESSION_BORDER_COLOR));


        


        // Draw Track Pan and Buttons

        auto muted = track->mute.get();
        auto solo = track->solo.get();
        auto armed = track->arm.get();
        float pan = track->panNode->pan()->value();
        auto panMin = track->panNode->pan()->minValue();
        auto panMax = track->panNode->pan()->maxValue();

        // Draw pan
        SetCursorScreenPos(ImVec2(startCenter_x, startVCenterButtons_y));
        PushID( ("trackPan_" + std::to_string(i)).c_str() );
        PushStyleColor(ImGuiCol_ButtonActive, U32FromHex(TEXT_LIGHT_COLOR));
        PushStyleColor(ImGuiCol_ButtonHovered, U32FromHex(BOOL_ON_COLOR));
        if (ImGuiKnobs::Knob("Pan", &pan, -1.f, panMax, 0.f, "%.2f",
                             ImGuiKnobVariant_Tick, 50.f)) {
          track->panNode->pan()->setValueAtTime(pan, 0.f);
        }
        PopStyleColor(2);
        PopID();

        // Draw Mute
        if(LetterButton(draw_list, "M", ImVec2(startCenter_x, startVCenterButtons_y + panElementHeight + (trackButtonSize * 0) + (padding_y * 0)),trackButtonSize, muted)){
          track->mute.set(!muted);
        }

        // Draw Solo
        if(LetterButton(draw_list, "S", ImVec2(startCenter_x, startVCenterButtons_y + panElementHeight + (trackButtonSize * 1) + (padding_y * 1)), trackButtonSize, solo, false, U32FromHex(SOLO_COLOR))) {
          track->solo.set(!solo);
        }

        // Draw Arm
        if (LetterButton(draw_list, "R", ImVec2(startCenter_x, startVCenterButtons_y + panElementHeight + (trackButtonSize * 2) + (padding_y * 2)), trackButtonSize, armed, false, U32FromHex(RECORD_COLOR))) {
          track->arm.set(!armed);
        }

        // Draw Meters
        auto volumeLabel = (track->name.value + "_volumeSlider");
        auto meterStart_x = startCenter_x + trackButtonSize + padding_x;
        float volume = track->volumeNode->gain()->value();
        float rmsL = track->meterNode->rmsDb()[0];
        float rmsR = track->meterNode->rmsDb()[1];
        float peakL = track->meterNode->db()[0];
        float peakR = track->meterNode->db()[1];
        StereoMeter(draw_list, ImVec2(meterStart_x, meterStart_y), ImVec2(12.f,meterSize_y), peakL, peakR, rmsL, rmsR);

        // Draw Slider
        // TODO: Extract Slider into it's own function. implement conversion formula to and from decibel and linear values. Also implement scale beside meter
        SetCursorScreenPos(ImVec2(meterStart_x + padding_x + 15.f, meterStart_y));
        PushID( ( track->name.value + std::to_string(i) ).c_str() );
        if (VSliderFloat("##v", ImVec2(sliderWidth, meterSize_y), &volume,
                         0.f, 1.5f))
        {
          track->volumeNode->gain()->setValueAtTime(volume, 0.0f);
        }
        PopID();

         // Draw bottom section rect
        draw_list->AddRectFilled(bottomSectionMin, bottomSectionMax, trackFooterBgColor);
         // Draw Bottom Section text
        auto label = track->name.value.c_str();
        ImVec2 text_size = ImGui::CalcTextSize(label);
        ImVec2 text_pos = ImVec2(bottomSectionMin.x + (trackWidth - text_size.x) * 0.5f,
                                 bottomSectionMin.y + (bottomSectionHeight - text_size.y) * 0.5f);
        draw_list->AddText(text_pos, trackFooterTextColor, label);
        if(IsMouseHit(bottomSectionMin, bottomSectionMax, io.MousePos) && IsMouseClicked(0)){
          session->selectTrack(i);
        }

        // Draw Track Border
        draw_list->AddRect(ImVec2(start_x, position.y),
                           ImVec2(end_x, position.y + size.y),
                           U32FromHex(SESSION_BORDER_COLOR), 0.f);
    }
  EndChild();
  PopStyleVar();
  PopStyleColor();
}

#endif // !TRACKLIST_H