#ifndef GRIDVIEW_H
#define GRIDVIEW_H
#include <imgui.h>


class GridView{

public:
    ImVec2 size;
    ImVec2 origin;
    ImU32 gridLineColor;
    ImDrawList *draw_list;
};

#endif // !GRIDVIEW_H