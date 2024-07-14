#ifndef STEREOMETER_H
#define STEREOMETER_H

#include "imgui.h"
#include "ui/Colors.h"

using namespace ImGui;

void StereoMeter(
    ImDrawList *draw_list,
    ImVec2 position,
    ImVec2 size,
    float peakLeftValue,
    float peakRightValue,
    float rmsLeftValue,
    float rmsRightValue,
    float peakMin = -72.f,
    float peakMax = 0.f,
    float rmsMin = -72.f,
    float rmsMax = 0.f)
{

    // Define measurements;
    float meterSpacing = 2.f;
    float meterWidth = (size.x - meterSpacing) / 2.f;
    auto meterLeftStart_x = position.x;
    auto meterLeftEnd_x = position.x + meterWidth;
    auto meterRightStart_x = meterLeftEnd_x + meterSpacing;
    auto meterRightEnd_x = meterRightStart_x + meterWidth;
    auto meterStart_y = position.y;
    auto meterSize_y = size.y;
    auto meterEnd_y = meterStart_y + meterSize_y;

    // Calculate rms and peak
    auto peakL = (1 - peakLeftValue / (peakMin - peakMax)) * meterSize_y;
    auto peakR = (1 - peakRightValue / (peakMin - peakMax)) * meterSize_y;
    auto rmsL = (1 - rmsLeftValue / (rmsMin - rmsMax)) * meterSize_y;
    auto rmsR = (1 - rmsRightValue / (rmsMin - rmsMax)) * meterSize_y;

    //  Define Colors
    auto rdMtrClr = U32FromHex(METER_RED_COLOR);
    auto tpMtrClr = U32FromHex(METER_TOP_COLOR);
    auto midMtrClr = U32FromHex(METER_MIDDLE_COLOR);
    auto btmMrtClr = U32FromHex(METER_BOTTOM_COLOR);

    //  Draw Background Left
    draw_list->AddRectFilled(ImVec2(meterLeftStart_x, meterStart_y), ImVec2(meterLeftEnd_x + meterWidth, meterEnd_y), U32FromHex(TEXT_DARK_COLOR));

    //  Draw Background Right
    draw_list->AddRectFilled(ImVec2(meterRightStart_x, meterStart_y), ImVec2(meterRightEnd_x, meterEnd_y), U32FromHex(TEXT_DARK_COLOR));

    // Left Meter bars
    draw_list->PushClipRect(ImVec2(meterLeftStart_x, meterEnd_y - rmsL), ImVec2(meterLeftEnd_x, meterEnd_y));
    draw_list->AddRectFilledMultiColor(ImVec2(meterLeftStart_x, meterStart_y), ImVec2(meterLeftEnd_x, meterStart_y + (meterSize_y * .25f)), rdMtrClr, rdMtrClr, tpMtrClr, tpMtrClr);
    draw_list->AddRectFilledMultiColor(ImVec2(meterLeftStart_x, meterStart_y + (meterSize_y * .25f)), ImVec2(meterLeftEnd_x, meterStart_y + (meterSize_y * .5f)), tpMtrClr, tpMtrClr, midMtrClr, midMtrClr);
    draw_list->AddRectFilledMultiColor(ImVec2(meterLeftStart_x, meterStart_y + (meterSize_y * .5f)), ImVec2(meterLeftEnd_x, meterEnd_y), midMtrClr, midMtrClr, btmMrtClr, btmMrtClr);
    draw_list->PopClipRect();

    // Right Meter bars
    draw_list->PushClipRect(ImVec2(meterRightStart_x, meterEnd_y - rmsR), ImVec2(meterRightEnd_x, meterEnd_y));
    draw_list->AddRectFilledMultiColor(ImVec2(meterRightStart_x, meterStart_y), ImVec2(meterRightEnd_x, meterStart_y + (meterSize_y * .25f)), rdMtrClr, rdMtrClr, tpMtrClr, tpMtrClr);
    draw_list->AddRectFilledMultiColor(ImVec2(meterRightStart_x, meterStart_y + (meterSize_y * .25f)), ImVec2(meterRightEnd_x, meterStart_y + (meterSize_y * .5f)), tpMtrClr, tpMtrClr, midMtrClr, midMtrClr);
    draw_list->AddRectFilledMultiColor(ImVec2(meterRightStart_x, meterStart_y + (meterSize_y * .5f)), ImVec2(meterRightEnd_x, meterEnd_y), midMtrClr, midMtrClr, btmMrtClr, btmMrtClr);
    draw_list->PopClipRect();

    draw_list->PushClipRect(ImVec2(meterLeftStart_x, meterStart_y), ImVec2(meterRightEnd_x, meterEnd_y));
    draw_list->AddLine(ImVec2(meterLeftStart_x, meterEnd_y - peakL), ImVec2(meterLeftEnd_x, meterEnd_y - peakL), U32FromHex(WHITE_COLOR));
    draw_list->AddLine(ImVec2(meterRightStart_x, meterEnd_y - peakR), ImVec2(meterRightEnd_x, meterEnd_y - peakR), U32FromHex(WHITE_COLOR));
    draw_list->PopClipRect();
}

#endif // !STEREOMETER_H