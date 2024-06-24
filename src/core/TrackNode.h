#ifndef TRACKNODE_H
#define TRACKNODE_H

#include "LabSound//LabSound.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/core/AudioNode.h"
#include "LabSound/core/GainNode.h"
#include "LabSound/core/StereoPannerNode.h"
#include <memory>
#include <string>
#include "uuid.h"

using namespace lab;
using std::shared_ptr;
using std::make_shared;
using std::string;

class TrackNode
{
public:
    string name;
    uuids::uuid id;
    shared_ptr<GainNode> volume;
    shared_ptr<StereoPannerNode> pan;
    shared_ptr<AudioContext> context;
TrackNode(shared_ptr<AudioContext> context)
    : context(context)
{
    volume = make_shared<GainNode>(*this->context);
    pan = make_shared<StereoPannerNode>(*this->context);
    this->context->connect(volume, pan);
    this->context->connect(this->context->destinationNode(), volume);
}
};

#endif