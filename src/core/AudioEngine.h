#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H
#include "LabSound/LabSound.h"
#include "LabSound/backends/AudioDevice_RtAudio.h"
#include "LabSound/core/AudioDevice.h"
#include <memory>
using namespace lab;
using std::vector;
using std::shared_ptr;
namespace tstudio
{
    class AudioEngine
    {
    private:
        /* data */
    public:
      AudioStreamConfig inputConfig;
      AudioStreamConfig outputConfig;
      vector<AudioDeviceInfo> audioDevices;
      AudioDeviceInfo defaultOutputInfo, defaultInputInfo;
      shared_ptr<AudioDevice_RtAudio> audioDevice;
      shared_ptr<AudioContext> context;
      shared_ptr<AudioDestinationNode> destinationNode;
      AudioEngine(/* args */);
      ~AudioEngine();
      void init();
      shared_ptr<AudioContext> activate();
      void deactivate();
    };

}

#endif