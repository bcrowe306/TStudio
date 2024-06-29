#ifndef INSTRUMENTDEVICE_H
#define INSTRUMENTDEVICE_H
#include "core/MidiMsg.h"
#include "core/MidiNode.h"
#include "LabSound/LabSound.h"
#include <memory>

using std::shared_ptr;
using std::make_shared;
using namespace lab;

namespace tstudio {

    // An interface that defines an InstrumentDevice. Must implement onMidiMsg
    class InstrumentDevice : public MidiNode {
    public:
      shared_ptr<AnalyserNode> output;
      shared_ptr<AudioContext> context;

      InstrumentDevice() = delete;
      explicit InstrumentDevice(shared_ptr<AudioContext> context) {
        output = make_shared<AnalyserNode>(*context);
      };
      virtual ~InstrumentDevice() = default;

    };

}

#endif // !INSTRUMENTDEVICE_H