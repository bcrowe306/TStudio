#ifndef LETTERBUTTON_H
#define LETTERBUTTON_H

#include "imgui.h"
#include "ui/Colors.h"
#include "ui/Utility.h"
#include <string>

using namespace ImGui;

bool LetterButton(
    ImDrawList *draw_list,
    std::string letter,
    ImVec2 position,
    float size,
    bool active = false,
    bool disabled = false,
    ImU32 activeColor = U32FromHex(BUTTON_COLOR_ACTIVE),
    ImU32 defaultColor = U32FromHex(BUTTON_COLOR_DEFAULT),
    ImU32 disabledColor = U32FromHex(TEXT_LIGHT_COLOR))
{
    auto buttonMin = position;
    auto buttonMax = ImVec2(buttonMin.x + size, buttonMin.y + size);
    ImU32 buttonColor;
    ImU32 textColor;

    if (!disabled)
    {
        if (active)
        {
            buttonColor = activeColor;
            textColor = U32FromHex(TEXT_DARK_COLOR);
        }
        else
        {
            buttonColor = defaultColor;
            textColor = activeColor;
        }
    }
    else
    {
        buttonColor = disabledColor;
    }
    draw_list->AddRectFilled(buttonMin, buttonMax, buttonColor);
    draw_list->AddRect(buttonMin, buttonMax, U32FromHex(SESSION_BORDER_COLOR));
    ImVec2 text_size = ImGui::CalcTextSize(letter.c_str());
    auto textStart_x = position.x + ((size - text_size.x) / 2);
    auto textStart_y = position.y + ((size - text_size.y) / 2);
    draw_list->AddText(ImVec2(textStart_x, textStart_y), textColor, letter.c_str());
    bool clicked = false;
    ImGuiIO &io = ImGui::GetIO();
    if (ImGui::IsMouseClicked(0) && IsMouseHit(buttonMin, buttonMax, io.MousePos))
    {
        clicked = true;
    }
    return clicked;
}

#endif // !LETTERBUTTON_H