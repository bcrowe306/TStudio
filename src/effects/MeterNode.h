// License: BSD 2 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef METERNODE_H
#define METERNODE_H

#include "core/AudioUtility.h"
#include "LabSound/LabSound.h"

using namespace lab;

namespace tstudio {

    class MeterNode : public AudioBasicInspectorNode {


public:

    // Members
    int windowSize = 128;
    float smapleAmount = -10000.f;
    float maxLinear = 0.f;
    float maxDb = 30.f;
    float maxPercentage = 0.f;

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
    float percentage(){
        auto max = dBToLinear(maxDb);
        auto perc =  (dBToLinear(_db) / max);
        if(perc > maxPercentage){
            maxPercentage = perc;
        }
        return perc;
    };
    float dbAsLinear() { 
        auto linear = std::pow(10.f, 0.05f * _db);
        if (linear > maxLinear) {
          maxLinear = linear;
        }
        return linear;
    }

  private:
    float _db;

};

}




#endif // METERNODE_H
