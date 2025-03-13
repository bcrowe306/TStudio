#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <atomic>

class AudioNode
{
public:
    using Ptr = std::shared_ptr<AudioNode>;

    AudioNode(size_t maxFrameCount = 2) : processCycle(0), cachedSamples(maxFrameCount, 0.0f) {}
    virtual ~AudioNode() = default;

    void addInputNode(Ptr node)
    {
        std::lock_guard<std::mutex> lock(graphMutex);
        inputNodes.push_back(std::move(node));
    }

    void addOutputNode(Ptr node)
    {
        std::lock_guard<std::mutex> lock(graphMutex);
        outputNodes.push_back(std::move(node));
    }

    void removeInputNode(Ptr node)
    {
        std::lock_guard<std::mutex> lock(graphMutex);
        inputNodes.erase(std::remove(inputNodes.begin(), inputNodes.end(), node), inputNodes.end());
    }

    void removeOutputNode(Ptr node)
    {
        std::lock_guard<std::mutex> lock(graphMutex);
        outputNodes.erase(std::remove(outputNodes.begin(), outputNodes.end(), node), outputNodes.end());
    }

    void process(float *input, float *output, unsigned long frameCount, std::atomic<uint64_t> &cycle)
    {
        std::lock_guard<std::mutex> lock(processMutex);

        if (lastProcessCycle == cycle.load())
        {
            std::copy(cachedSamples.begin(), cachedSamples.begin() + frameCount, output);
            return;
        }

        lastProcessCycle = cycle.load();
        processAudio(input, cachedSamples.data(), frameCount);
        std::copy(cachedSamples.begin(), cachedSamples.begin() + frameCount, output);
    }

protected:
    virtual void processAudio(float *input, float *output, unsigned long frameCount)
    {
        for (auto &node : inputNodes)
        {
            node->process(input, output, frameCount, processCycle);
        }
    }

private:
    std::vector<Ptr> inputNodes;
    std::vector<Ptr> outputNodes;
    std::vector<float> cachedSamples;
    std::mutex graphMutex;
    std::mutex processMutex;
    std::atomic<uint64_t> processCycle;
    uint64_t lastProcessCycle = 0;
};
