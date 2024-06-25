#include "core/AudioEngine.h"
#include "LabSound/LabSound.h"
#include <memory>

using namespace lab;
using std::make_shared;
namespace tstudio {

    AudioEngine::AudioEngine(/* args */) {
        init();
    };

    AudioEngine::~AudioEngine() {};
    void AudioEngine::init() {
        audioDevices = lab::AudioDevice_RtAudio::MakeAudioDeviceList();
        bool with_input = false;

        for (auto &info : audioDevices) {
            if (info.is_default_output)
                defaultOutputInfo = info;
            if (info.is_default_input)
                defaultInputInfo = info;
        }
        // Create Output Stream Config with desired defaults
        if (defaultOutputInfo.index != -1) {
            outputConfig.device_index = defaultOutputInfo.index;
            outputConfig.desired_channels =
                std::min(uint32_t(2), defaultOutputInfo.num_output_channels);
            outputConfig.desired_samplerate =
                defaultOutputInfo.nominal_samplerate;
        }
        // Create Input Stream Config with desired defaults
        if (with_input) {
            if (defaultInputInfo.index != -1) {
            inputConfig.device_index = defaultInputInfo.index;
            inputConfig.desired_channels =
                std::min(uint32_t(1), defaultInputInfo.num_input_channels);
            inputConfig.desired_samplerate =
                defaultInputInfo.nominal_samplerate;
            } else {
            throw std::invalid_argument("the default audio input device was "
                                        "requested but none were found");
            }
        }

        // RtAudio doesn't support mismatched input and output rates.
        // this may be a pecularity of RtAudio, but for now, force an RtAudio
        // compatible configuration
        if (defaultOutputInfo.nominal_samplerate !=
            defaultInputInfo.nominal_samplerate) {
            float min_rate = std::min(defaultOutputInfo.nominal_samplerate,
                                      defaultInputInfo.nominal_samplerate);
            inputConfig.desired_samplerate = min_rate;
            outputConfig.desired_samplerate = min_rate;
            printf("Warning ~ input and output sample rates don't match, "
                   "attempting to "
                   "set minimum\n");
        }
    };
    shared_ptr<AudioContext> AudioEngine::activate() {
        // Create audio device using RTAudio Backend
        audioDevice = make_shared<AudioDevice_RtAudio>(inputConfig, outputConfig);

        // Create the audio context
        context = std::make_shared<AudioContext>(false, true);
        destinationNode = std::make_shared<lab::AudioDestinationNode>(
            *context.get(), audioDevice);
        audioDevice->setDestinationNode(destinationNode);
        context->setDestinationNode(destinationNode);
        context->synchronizeConnections();
        return context;
    };
    void AudioEngine::deactivate(){
        // device, context, and rendernode are circularly referenced, so break
        // the cycle manually.
        context->disconnect(context->destinationNode());
        context->synchronizeConnections();
        auto destinationNode = context->destinationNode();
        destinationNode.reset();
        audioDevice->setDestinationNode(destinationNode);
        context->setDestinationNode(destinationNode);
    }
} // namespace tstudio
