#ifndef UTILITY_H
#define UTILITY_H

#include <imgui.h>

bool IsMouseHit(ImVec2 rectMin, ImVec2 rectMax, ImVec2 mousePosition){
    return mousePosition.x > rectMin.x && mousePosition.x < rectMax.x && mousePosition.y > rectMin.y && mousePosition.y < rectMax.y;
}
void DrawTriangle(ImVec2 triangleSize){}

#endif // !UTILITY_H