#ifndef EVENTREGISTRY_H
#define EVENTREGISTRY_H

#include <algorithm>
#include <any>
#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace tstudio {

class EventRegistry {
public:
  using HandlerId = std::size_t;

  // Singleton instance accessor
  static EventRegistry &getInstance() {
    static EventRegistry instance;
    return instance;
  }

  // Subscribe to an event with a specific event ID
  HandlerId subscribe(const std::string &event_id,
                      const std::function<void(std::any)> &handler) {
    std::lock_guard<std::mutex> lock(mtx);
    HandlerId id = nextHandlerId++;
    handlers[event_id].emplace_back(id, handler);
    return id;
  }

  // Unsubscribe from an event with a specific event ID
  void unsubscribe(const std::string &event_id, HandlerId id) {
    std::lock_guard<std::mutex> lock(mtx);
    auto &handlerList = handlers[event_id];
    handlerList.erase(
        std::remove_if(handlerList.begin(), handlerList.end(),
                       [id](const auto &pair) { return pair.first == id; }),
        handlerList.end());
    if (handlerList.empty()) {
      handlers.erase(event_id);
    }
  }

  // Notify all subscribers of a specific event with data
  void notify(const std::string &event_id, std::any data) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = handlers.find(event_id);
    if (it != handlers.end()) {
      for (const auto &[id, handler] : it->second) {
        handler(data);
      }
    }
  }

private:
  std::unordered_map<
      std::string,
      std::vector<std::pair<HandlerId, std::function<void(std::any)>>>>
      handlers;
  std::mutex mtx;
  std::atomic<HandlerId> nextHandlerId{0};

  // Private constructor to prevent instantiation
  EventRegistry() {}

  // Delete copy constructor and assignment operator
  EventRegistry(const EventRegistry &) = delete;
  EventRegistry &operator=(const EventRegistry &) = delete;
};

class EventBase {
  public:
  EventRegistry &eventRegistry;
  EventBase() : eventRegistry(EventRegistry::getInstance()){}
};
}



#endif // !EVENTREGISTRY_H

