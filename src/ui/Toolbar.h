#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <memory>
#include "core/Session.h"
#include "core/Playhead.h"
#include "imgui.h"
#include "ui/Colors.h"

using namespace tstudio;
using std::shared_ptr;

void Toolbar (shared_ptr<Session> session, shared_ptr<Playhead> playhead, ImVec2 toolbarPosition, ImVec2 toolbarSize){
    // Toolbar Panel
    ImGui::SetNextWindowPos(toolbarPosition);
    ImGui::SetNextWindowSize(toolbarSize);
    ImGui::Begin("Toolbar", NULL,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoDecoration);
    
    static std::pair<int, int> sessionPosition;

    ImGui::Dummy(ImVec2(240.f, toolbarSize.y)); ImGui::SameLine();
    
    // Tempo Drag
    auto tempo = playhead->getTempo();
    ImGui::AlignTextToFramePadding();
    ImGui::PushItemWidth(50.f);
    float one = 1.f;
    auto tempoText = ImGui::DragFloat(": BPM", &tempo, 1.f, 30.f, 300.f, "%.2f");
    if (tempoText)
    {
        if (tempo > 30 && tempo < 300)
        {

            playhead->setTempo(tempo);
        }
    }
    ImGui::SameLine();
    ImGui::Text("%02d:%02d:%03d", playhead->songPosition.bar + 1, playhead->songPosition.beat + 1, playhead->songPosition.tick + 1);
    ImGui::SameLine();

    // Metronome Enable
    auto metronomeEnabled = playhead->getMetronomeEnabled();
    bool metronomeWidget = ToggleWidget("Metronome", &metronomeEnabled, ImVec2(80.f, 20.f));
    ImGui::SameLine();
    if (metronomeWidget)
    {
        playhead->setMetronomeEnabled(metronomeEnabled);
    }

    ImU32 playbuttonColor;
    ImU32 recordButtonColor;
    float playButtonHue = 0.35f;
    switch (playhead->getState())
    {
    case PlayheadState::RECORDING:
        playButtonHue = 0.35f;
        playbuttonColor = U32FromHex(PLAY_COLOR);
        recordButtonColor = U32FromHex(RECORD_COLOR);
        break;
    case PlayheadState::PRECOUNT:
        playButtonHue = 0.35f;
        playbuttonColor = U32FromHex(PLAY_COLOR);
        recordButtonColor = U32FromHex(LAUNCHING_RECORDING_COLOR);
        break;
    case PlayheadState::PLAYING:
        playButtonHue = 0.35f;
        playbuttonColor = U32FromHex(PLAY_COLOR);
        recordButtonColor = U32FromHex(TEXT_LIGHT_COLOR);
        break;
    case PlayheadState::STOPPED:
        playButtonHue = 0.66;
        playbuttonColor = U32FromHex(TEXT_LIGHT_COLOR);
        recordButtonColor = U32FromHex(TEXT_LIGHT_COLOR);
        break;
    default:
        playButtonHue = 0.66;
        break;
    }

    // Play Button
    auto triangleSize = 20.f;
    auto playButton = ImGui::InvisibleButton("playbuttonFunctionality", ImVec2(triangleSize, toolbarSize.y));
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    auto p = ImGui::GetItemRectMin();
    auto left_padding = 0.f;
    auto start_y = p.y +5.f;
    auto trigger_start = ImVec2(p.x + left_padding, start_y);
    auto trigger_middle =
        ImVec2(trigger_start.x + triangleSize,
               trigger_start.y + triangleSize / 2);
    auto trigger_end =
        ImVec2(trigger_start.x, trigger_start.y + triangleSize);
    draw_list->AddTriangleFilled(trigger_start, trigger_middle,
                                 trigger_end, playbuttonColor);
    ImGui::SameLine();
    if (playButton)
    {
        playhead->togglePlay();
    }
    // ImGui::PopID();
    ImGui::SameLine();

    ImGui::Dummy(ImVec2(5.f, toolbarSize.y)); ImGui::SameLine();
    // RecordButton
    auto recordButtonSize = 20.f;
    auto recordButton = ImGui::InvisibleButton("recordButton", ImVec2(recordButtonSize, toolbarSize.y));
    auto leftPadding = 0.f;
    auto recordButtonPosition = ImGui::GetItemRectMin();
    auto circle_center = ImVec2(
        recordButtonPosition.x + recordButtonSize / 2, recordButtonPosition.y + 15.f);
    auto circle_radius = recordButtonSize / 2;
    draw_list->AddCircleFilled(circle_center, circle_radius,
                               recordButtonColor, 0.f);
    if (recordButton)
    {
        playhead->toggleRecord();
    }
    ImGui::SameLine();

    ImGui::Dummy(ImVec2(5.f, toolbarSize.y)); ImGui::SameLine();

    // Draw stop element
    auto stopButtonSize = 20.f;
    auto stopButton = ImGui::InvisibleButton("stopButton", ImVec2(stopButtonSize, toolbarSize.y));
    auto stopButtonMinPos = ImGui::GetItemRectMin();
    auto stopButtonStart_y = stopButtonMinPos.y + 5.f;
    auto stopButtonStart = ImVec2(stopButtonMinPos.x, stopButtonStart_y);
    auto stopButtonEnd = ImVec2(stopButtonStart.x + stopButtonSize,
                              stopButtonStart.y + stopButtonSize);
    draw_list->AddRectFilled(stopButtonStart, stopButtonEnd, U32FromHex(TEXT_LIGHT_COLOR),
                             0.f);
    if (stopButton)
    {
        playhead->stop();
    }
    ImGui::SameLine();


    ImGui::SameLine();
    if (ImGui::BeginPopupContextItem("toolbarContext"))
    {
        if (ImGui::MenuItem("Add Track"))
            session->addTrack();
        ImGui::MenuItem("Copy");
        if (ImGui::MenuItem("Add Scene"))
            session->addScene();
        ImGui::MenuItem("Duplicate");
        ImGui::EndPopup();
    }
    ImGuiIO &io = ImGui::GetIO();
    if (ImGui::IsMouseClicked(1) && IsMouseHit(toolbarPosition, ImVec2(toolbarPosition.x + toolbarSize.x, toolbarPosition.y + toolbarSize.y), io.MousePos))
    {
        ImGui::OpenPopup("toolbarContext");
    }
    

    ImGui::End();
};

#endif // !TOOLBAR_H