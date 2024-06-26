#include "display/Widget.h"
#include <cairo.h>

class TextWidget : public Widget
{
private:
    /* data */
public:
    TextWidget
(/* args */);
    ~TextWidget
();
    // Override the draw method
    void draw(cairo_t *cr) override {
        drawText(cr, "This is a test", 0, 0);
    };
};

TextWidget::TextWidget(/* args */)
{
}

TextWidget::~TextWidget()
{
}
