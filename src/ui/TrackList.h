#ifndef TRACKLIST_H
#define TRACKLIST_H

#include "core/Playhead.h"
#include "core/Session.h"
#include "imgui.h"
#include "ui/Colors.h"
#include "ui/LayoutDimensions.h"
#include "ui/SessionCell.h"
#include "ui/TrackHeader.h"
#include <memory>

using namespace tstudio;
using namespace ImGui;

using std::shared_ptr;

void TrackList(shared_ptr<Session> session, shared_ptr<Playhead> playHead, ImVec2 position, ImVec2 size){
  // Testing TrackList
  float trackListHeight = size.y;
  float trackListWidth = size.x;
  float track_width = 145.f;
  float track_name_height = 20.f;
  float meter_width = 5.f;
  float meter_spacing = 7.f;
  auto draw_list = GetWindowDrawList();
  SetNextWindowPos(position);
  PushStyleVar(ImGuiStyleVar_ChildRounding, 0.f);
  PushStyleColor(ImGuiCol_ChildBg, U32FromHex(SESSION_BACKGROUND_COLOR));
  BeginChild("tracklist", size, ImGuiChildFlags_Border);
    for (int i = 0; i < session->tracks.size(); i++)
    {
        auto track = session->tracks[i];
        auto start_x = position.x + (track_width * i);
        auto end_x = position.x + (track_width * i) + track_width;
        auto trackName_start = ImVec2(start_x, position.y + size.y - track_name_height);
        auto trackName_end = ImVec2(end_x, position.y + size.y);
        draw_list->AddRectFilled(trackName_start, trackName_end, U32FromHex(track->color.value.c_str()));

        // Draw text on top of the rectangle
        auto label = track->name.value.c_str();
        ImVec2 text_size = ImGui::CalcTextSize(label);
        ImVec2 text_pos = ImVec2(trackName_start.x + (track_width - text_size.x) * 0.5f,
                                 trackName_start.y + (track_name_height - text_size.y) * 0.5f);
        draw_list->AddText(text_pos, U32FromHex(SESSION_BORDER_COLOR), label);

        // Draw Track Border
        draw_list->AddRect(ImVec2(start_x, position.y), ImVec2(end_x, position.y + size.y), U32FromHex(SESSION_BORDER_COLOR), 0.f);

        // Draw Meters
        auto volumeLabel = (track->name.value + "_volumeSlider");
        auto meter_start_x = start_x + track_width / 2;
        draw_list->AddRectFilled(ImVec2(meter_start_x, position.y + 15.f), ImVec2(meter_start_x + meter_width, position.y + size.y -track_name_height - 15.f ), U32FromHex(PLAY_COLOR));
        draw_list->AddRectFilled(ImVec2(meter_start_x + meter_spacing, position.y + 15.f), ImVec2(meter_start_x + meter_width + meter_spacing, position.y + size.y -track_name_height - 15.f ), U32FromHex(PLAY_COLOR));
        auto volume = track->volumeNode->gain()->value();
        SetCursorScreenPos( ImVec2(meter_start_x + meter_spacing + 15.f, position.y + 15.f) );
        if (VSliderFloat(volumeLabel.c_str(), ImVec2(15.f, 200.f), &volume,
                         track->volumeNode->gain()->minValue(), 4.f)) {
          track->volumeNode->gain()->setValueAtTime(volume, 0.01f);
        }
        auto db = std::to_string(track->meterNode->db()).c_str();
        Text(db);
    }
  EndChild();
  PopStyleVar();
  PopStyleColor();
}

#endif // !TRACKLIST_H