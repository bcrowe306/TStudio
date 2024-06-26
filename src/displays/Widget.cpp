// Widget.cpp

#include "Widget.h"



// Constructor
Widget::Widget() {}

// Destructor
Widget::~Widget()
{
    for (auto child : children)
    {
        delete child;
    }
    children.clear();
}

// Add a child widget
void Widget::addChild(Widget *child)
{
    if (child)
    {
        children.push_back(child);
    }
}

// Process method to draw children and the widget itself
void Widget::process(cairo_t *cr)
{
    // Draw all child widgets first
    for (auto child : children)
    {
        child->process(cr);
    }
    // Draw the widget itself
    this->draw(cr);
}
