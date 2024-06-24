#pragma once

#include "core/MidiMsg.h"

namespace tstudio
{

    class MidiHandler
    {
    protected:
        MidiHandler *nextHandler;

    public:
        MidiHandler() : nextHandler(nullptr) {}

        void setNextHandler(MidiHandler *handler)
        {
            nextHandler = handler;
        }

        void handle(MidiMsg &event)
        {
            process(event);
            if (!event.isHandled && nextHandler)
            {
                nextHandler->handle(event);
            }
        }

        virtual void process(MidiMsg &event) = 0;
    };
}

