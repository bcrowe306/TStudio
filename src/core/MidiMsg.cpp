#include "core/MidiMsg.h"

using namespace std;
using choc::midi::ShortMessage;

namespace tstudio{
    MidiMsg::MidiMsg(const void *sourceData, size_t numBytes, optional<string> device, optional<int> index)
        : ShortMessage(sourceData, numBytes),
          device(device), index(index){};

    MidiMsg::~MidiMsg()
    {
    }
}

