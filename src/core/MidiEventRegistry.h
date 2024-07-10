#pragma once
#include <iostream>
#include <vector>
#include <functional>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <mutex>
#include <string>
#include <atomic>
#include "core/MidiMsgFilter.h"
#include "core/MidiMsg.h"

using namespace std;

namespace tstudio {
  struct MidiListener
  {
    std::function<void(MidiMsg &)> handler;
    MidiMsgFilter filter;
  };

  class MidiEventRegistry
  {
  public:
    using HandlerId = int;

    // Singleton instance accessor
    static MidiEventRegistry &getInstance()
    {
      static MidiEventRegistry instance;
      return instance;
    }

    int subscribe(MidiMsgFilter &filter, const std::function<void(MidiMsg &)> &handler)
    {
      auto newHandlerId = nextHandlerId++;
      auto result = handlers[newHandlerId] =  MidiListener{handler, filter};
      return newHandlerId;
    }

    // Unsubscribe from an event with a specific event ID
    void unsubscribe(int handlerId)
    {
      auto results = handlers.erase(handlerId);
    }

    // Notify all subscribers of a specific event with data
    void notify(const std::string &event_id, MidiMsg &midiMsg)
    {
      for (const auto &[handlerId, midiListener] : handlers)
      {
        auto filter = midiListener.filter;
        auto handler = midiListener.handler;
        if (filter.match(midiMsg))
        {
          handler(midiMsg);
        }
      }
    }

  private:
    std::mutex mtx;
    vector<MidiListener> listeners;
    unordered_map<int, MidiListener> handlers;
    int nextHandlerId =0 ;

    // Private constructor to prevent instantiation
    MidiEventRegistry() {}

    // Delete copy constructor and assignment operator
    MidiEventRegistry(const MidiEventRegistry &) = delete;
    MidiEventRegistry &operator=(const MidiEventRegistry &) = delete;
  };
}
