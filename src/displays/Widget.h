// Widget.h

#ifndef WIDGET_H
#define WIDGET_H

#include <vector>
#include <cairo.h>
#include <string>
#include <array>
#include <stdexcept>
#include <sstream>
#include <iomanip>

#pragma once

class Widget
{
public:
    // Constructor
    Widget();

    // Destructor
    virtual ~Widget();

    // Method to draw the widget (must be implemented by derived classes)
    virtual void draw(cairo_t *cr) = 0;

    // Add a child widget
    void addChild(Widget *child);

    std::array<float, 3> hexToRgbFloat(const std::string &hexColor)
    {
        // Ensure the string starts with '#' and has exactly 7 characters
        if (hexColor[0] != '#' || hexColor.length() != 7)
        {
            throw std::invalid_argument("Invalid hex color format. Expected format is #RRGGBB.");
        }

        // Convert hex string to integers
        int r, g, b;
        std::stringstream ss;
        ss << std::hex << hexColor.substr(1, 2);
        ss >> r;
        ss.clear();
        ss << std::hex << hexColor.substr(3, 2);
        ss >> g;
        ss.clear();
        ss << std::hex << hexColor.substr(5, 2);
        ss >> b;

        // Convert to floats and normalize between 0 and 1
        return {r / 255.0f, g / 255.0f, b / 255.0f};
    }
    void drawText(cairo_t *cr, const char *text, double x, double y, double size = 24.0, const char * family = "Arial", std::string color = "#FF0000")
    {
        auto rgb = this->hexToRgbFloat(color);
        cairo_select_font_face(cr, family, CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, size);

        cairo_set_source_rgb(cr, rgb[0], rgb[1], rgb[2]);
        cairo_move_to(cr, x, y + size);
        cairo_show_text(cr, text);
    };

    // Process method to draw children and the widget itself
    void process(cairo_t *cr);

protected:
    // List of child widgets
    std::vector<Widget *> children;
};

#endif // WIDGET_H
