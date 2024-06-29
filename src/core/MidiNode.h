#ifndef MIDINODE_H
#define MIDINODE_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include "core/MidiMsg.h"
#include "library/UUID_Gen.h"
#include "library/Parameter.h"
#include "core/MidiNodeRegistry.h"
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::vector;

namespace tstudio
{
    class MidiNode : public std::enable_shared_from_this<MidiNode>
    {
    public:
        StringParam name = StringParam("name", "Name", "", "Text");
        uuids::uuid id;
        MidiNode() : id(GenerateUUID()) {}
        MidiNode(const string &name)
            : id(GenerateUUID())
        {
            this->name.set(name);
            MidiNodeRegistry::getInstance().registerNode(shared_from_this());
        }

        virtual ~MidiNode()
        {
            MidiNodeRegistry::getInstance().deregisterNode(shared_from_this());
        }

        // Push messages down to output nodes
        void push(MidiMsg &message);

        // Push message up to input nodes. Calls the same method on the parents input nodes. Stop propagation by returning true on the virtual  bool receive() method.
        void pushIn(MidiMsg &message);

        // Override this method to process/react/manipulate incoming messages from your input nodes
        virtual MidiMsg process(MidiMsg &message) { return message; };

        // Receive messages from child output nodes. This allows message to be sent back up the chain. Stop propagation of message by returning true.
        virtual bool receive(MidiMsg &message) { return false; };
        void addOutputNode(shared_ptr<MidiNode> node);
        void removeOutputNode(shared_ptr<MidiNode> node);
        const vector<shared_ptr<MidiNode>> &getOutputNodes() const { return outputNodes; }
        const vector<shared_ptr<MidiNode>> &getInputNodes() const { return inputNodes; }

    private:
        vector<shared_ptr<MidiNode>> outputNodes;
        vector<shared_ptr<MidiNode>> inputNodes;
    };
}

#endif // !MIDINODE_H
