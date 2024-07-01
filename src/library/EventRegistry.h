#ifndef EVENTREGISTRY_H
#define EVENTREGISTRY_H

#include <algorithm>
#include <any>
#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

using std::lock_guard;
using std::mutex;
using std::unique_lock;
using std::string;
using std::function;
using std::any;
using std::condition_variable;
using std::atomic;
using std::unordered_map;
using std::vector;
using std::pair;

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
  HandlerId subscribe(const string &event_id, const function<void(any)> &handler) {
    HandlerId id = nextHandlerId++;
    unique_lock ul(mtx);
    cv.wait(ul, [this, event_id](){return this->currentEventId != event_id;});
    handlers[event_id].emplace_back(id, handler);
    cv.notify_all();
    return id;
  }

  // Unsubscribe from an event with a specific event ID
  void unsubscribe(const string &event_id, HandlerId id) {
    lock_guard<mutex> lock(mtx);
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
  void notify(const string &event_id, any data) {
      // std::lock_guard<std::mutex> lock(mtx);
      currentEventId = event_id;
      auto it = handlers.find(event_id);
      if (it != handlers.end())
      {
        for (const auto &[id, handler] : it->second)
        {
          if (handler != nullptr)
          {
            handler(data);
          }
        }
      }
      currentEventId = "";
      cv.notify_all();
    }

private:
  string currentEventId = "";
  unordered_map<string, vector<pair<HandlerId, function<void(any)>>>> handlers;
  mutex mtx;
  atomic<HandlerId> nextHandlerId{0};
  condition_variable cv;
  
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

