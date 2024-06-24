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
    using HandlerId = std::size_t;

    // Singleton instance accessor
    static MidiEventRegistry &getInstance()
    {
      static MidiEventRegistry instance;
      return instance;
    }

    HandlerId subscribe(MidiMsgFilter &filter, const std::function<void(MidiMsg &)> &handler)
    {
      listeners.emplace_back(MidiListener{handler, filter});
    }

    // Unsubscribe from an event with a specific event ID
    void unsubscribe(const std::function<void(MidiMsg &)> &handler)
    {
      auto removal = std::remove_if(listeners.begin(), listeners.end(),
                                    [&handler](const MidiListener &midiListener)
                                    {
                                      return handler.target<void(MidiMsg &)>() == midiListener.handler.target<void(MidiMsg &)>();
                                    });

      listeners.erase(removal, listeners.end());
    }

    // Notify all subscribers of a specific event with data
    void notify(const std::string &event_id, MidiMsg &midiMsg)
    {
      for (const auto &midiListener : listeners)
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
    std::unordered_map<std::string, std::vector<std::pair<HandlerId, std::function<void(MidiMsg &)>>>> handlers;
    std::mutex mtx;
    vector<MidiListener> listeners;
    std::atomic<HandlerId> nextHandlerId{0};

    // Private constructor to prevent instantiation
    MidiEventRegistry() {}

    // Delete copy constructor and assignment operator
    MidiEventRegistry(const MidiEventRegistry &) = delete;
    MidiEventRegistry &operator=(const MidiEventRegistry &) = delete;
  };
}
