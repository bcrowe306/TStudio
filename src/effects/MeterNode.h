// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef METERNODE_H
#define METERNODE_H

#include "LabSound/LabSound.h"

using namespace lab;

namespace tstudio {

    class MeterNode : public AudioBasicInspectorNode {


public:

    // Members
    int windowSize = 128;
    float smapleAmount = .5f;

    // Contructors
    MeterNode(AudioContext &ac);
    virtual ~MeterNode();

    static const char *static_name() { return "Meter"; }
    virtual const char *name() const override { return static_name(); }
    static AudioNodeDescriptor *desc();

    // Methods
    virtual double tailTime(ContextRenderLock &r) const override { return 0; }
    virtual double latencyTime(ContextRenderLock &r) const override {
      return 0;
    }
    virtual void process(ContextRenderLock &, int bufferSize) override;
    virtual void reset(ContextRenderLock &) override;
    float db(){return _db;}

private:
    float _db;

};

}




#endif // METERNODE_H
