#include "LabSound/LabSound.h"
#include "effects/MeterNode.h"

using namespace lab;


namespace tstudio {

static AudioSettingDescriptor s_meterNodeSetting[] = {{nullptr}};

AudioNodeDescriptor *MeterNode::desc() {
  static AudioNodeDescriptor d{nullptr, s_meterNodeSetting, 1};
  return &d;
}

MeterNode::MeterNode(AudioContext &ac) : AudioBasicInspectorNode(ac, *desc()) {
  initialize();
}

MeterNode::~MeterNode() { uninitialize(); }

void MeterNode::process(ContextRenderLock &r, int bufferSize) {
  AudioBus *outputBus = output(0)->bus(r);
  AudioBus *inputBus = input(0)->bus(r);

  if (!isInitialized() || !input(0)->isConnected() || !inputBus) {
    if (outputBus)
      outputBus->zero();
    return;
  }

  // calculate the power of this buffer
  {
    int start = static_cast<int>(bufferSize) - static_cast<int>(windowSize);
    int end = static_cast<int>(bufferSize);
    if (start < 0)
      start = 0;

    float power = 0;
    int numberOfChannels = inputBus->numberOfChannels();
    for (int c = 0; c < numberOfChannels; ++c) {
      const float *data = inputBus->channel(c)->data();
      for (int i = start; i < end; ++i) {
        float p = data[i];
        power += p * p;
      }
    }
    float rms = sqrtf(power / (numberOfChannels * bufferSize));

    // Protect against accidental overload due to bad values in input stream
    const float kMinPower = 0.000125f;
    if (std::isinf(power) || std::isnan(power) || power < kMinPower)
      power = kMinPower;

    // db is 20 * log10(rms/Vref) where Vref is 1.0
    _db = 20.0f * logf(rms) / logf(10.0f);
  }
  // to here

  if (inputBus != outputBus) {
    outputBus->copyFrom(*inputBus);
  }
}

void MeterNode::reset(ContextRenderLock &) {}
}

