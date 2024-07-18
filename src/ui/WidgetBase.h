#ifndef WIDGETBASE_H
#define WIDGETBASE_H


#include <memory>
#include <vector>
#include <string>
#include <map>

namespace tstudio {

    class WidgetBase
    {
    public:
        
        WidgetBase();
        ~WidgetBase();
        virtual void draw() = 0;
        virtual void render();
    
    private:
        
    };
    
    WidgetBase::WidgetBase()
    {
    }
    
    WidgetBase::~WidgetBase()
    {
    }

}

#endif // !WidgetBase_H