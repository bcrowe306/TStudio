#ifndef MAIN_H
#define MAIN_H

#include <imgui.h>
#include "hello_imgui/hello_imgui.h"
#include <hello_imgui/app_window_params.h>
#include <hello_imgui/hello_imgui_font.h>
#include <hello_imgui/imgui_theme.h>
#include <hello_imgui/runner_params.h>
#include <hello_imgui/screen_bounds.h>
#include "ui/FooterPanel.h"
#include "ui/MainView.h"
#include "ui/Sidebar.h"
#include "ui/TrackListItem.h"
#include "ui/TrackHeader.h"
#include "ui/SessionCell.h"
#include "ui/GridView.h"
#include "ui/ClipView.h"
#include "ui/Toolbar.h"
#include "ui/Colors.h"
#include "ui/LayoutDimensions.h"

using namespace tstudio;
using namespace placeholders;

void mainUI(shared_ptr<Session> session, shared_ptr<Playhead> playhead)
{
    HelloImGui::RunnerParams params;
    params.appWindowParams.windowGeometry.fullScreenMode = HelloImGui::FullScreenMode::FullMonitorWorkArea;
    params.fpsIdling.fpsIdle = 40.f;
    params.callbacks.SetupImGuiStyle = [&]()
    {
        params.imGuiWindowParams.tweakedTheme.Theme = ImGuiTheme::ImGuiTheme_PhotoshopStyle;
        params.imGuiWindowParams.tweakedTheme.Tweaks.Rounding = 0.0f;
        params.imGuiWindowParams.tweakedTheme.Tweaks.RoundingScrollbarRatio = 0.0f;
    };
    params.callbacks.LoadAdditionalFonts = []()
    {
        HelloImGui::LoadFont("fonts/OpenSans.ttf", 16.f);
    };
    params.callbacks.ShowGui = [playhead, session]()
    {
        // Get the window size
        auto space = ImGui::GetContentRegionAvail();
        // auto screenSize = runnerParams->appWindowParams.windowGeometry.size;
        float window_width = space.x;
        float window_height = space.y;
        // Set up panel sizes and positions
        float toolbar_height = 50.0f;
        float sidebar_width = 250.0f;
        float footer_height = 300.0f;
        float main_width = window_width - sidebar_width;
        float main_height = window_height - footer_height - toolbar_height;
        ImGui::GetStyle().WindowRounding = 0.0f;
        ImGui::GetStyle().WindowBorderSize = 0.1f;
        ImGui::GetStyle().FrameRounding = 0.0f;
        ImGui::GetStyle().GrabRounding = 0.0f;
        ImGui::GetStyle().PopupRounding = 0.0f;
        ImGui::GetStyle().ScrollbarRounding = 0.0f;

        Toolbar(session, playhead, ImVec2(0, 0), ImVec2(window_width, toolbar_height));
        Sidebar(session, playhead, ImVec2(0, toolbar_height), ImVec2(sidebar_width, main_height));
        FooterPanel(session, playhead, ImVec2(0, toolbar_height + main_height), ImVec2(window_width, footer_height));
        MainView(session, playhead, ImVec2(sidebar_width, toolbar_height), ImVec2(main_width, main_height));
    };
    std::cout << "uiThread\n";
    return HelloImGui::Run(params);
};

#endif // !MAIN_H