#include <iostream>
#include "core/MidiNode.h"

namespace tstudio {

    void MidiNode::push(MidiMsg &message)
    {
        MidiMsg processedMessage = process(message);
        // std::string logMsg = "Node " + uuids::to_string(id) + " (" +  name.value + ") processed message: " + processedMessage.getDescription();
        
        for (auto &outputNode : outputNodes)
        {
            outputNode->push(processedMessage);
        }
    }

    void MidiNode::pushIn(MidiMsg &message)
    {
        if (!receive(message))
        {
            for (auto &inputNode : inputNodes)
            {
                inputNode->pushIn(message);
            }
        }
    }

   
    void MidiNode::addOutputNode(std::shared_ptr<MidiNode> node)
    {
        if (std::find(outputNodes.begin(), outputNodes.end(), node) == outputNodes.end())
        {
            outputNodes.push_back(node);
            node->inputNodes.push_back(shared_from_this());
        }
    }

    void MidiNode::removeOutputNode(std::shared_ptr<MidiNode> node)
    {
        auto it = std::find(outputNodes.begin(), outputNodes.end(), node);
        if (it != outputNodes.end())
        {
            outputNodes.erase(it);
            node->inputNodes.erase(std::remove(node->inputNodes.begin(), node->inputNodes.end(), shared_from_this()), node->inputNodes.end());
        }
    }
}



