#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <string>
#include <memory>
#include <atomic>
#include <stdexcept>

template <typename... Args>
class EventEmitter;

// HandlerWrapper Class
template <typename... Args>
class HandlerWrapper
{
public:
    using HandlerType = std::function<void(Args..., class EventEmitter<Args...> *)>;

    // Constructor
    HandlerWrapper(HandlerType handler)
        : id_(++nextId_), handler_(std::move(handler)) {}

    // Invoke the handler
    void operator()(Args... args, class EventEmitter<Args...> *source) const
    {
        handler_(args..., source);
    }

    // Get the unique ID
    size_t getId() const
    {
        return id_;
    }

    // Compare by unique ID
    bool operator==(const HandlerWrapper &other) const
    {
        return id_ == other.id_;
    }

private:
    size_t id_;
    HandlerType handler_;
    static std::atomic<size_t> nextId_;
};

template <typename... Args>
std::atomic<size_t> HandlerWrapper<Args...>::nextId_(0);

// EventEmitter Class
template <typename... Args>
class EventEmitter
{
public:
    using HandlerType = std::function<void(Args..., EventEmitter<Args...> *)>;
    using WrappedHandler = HandlerWrapper<Args...>;

    EventEmitter() = default;

    // Set the parent EventEmitter
    void setParent(EventEmitter *parent)
    {
        if (parent == this)
        {
            throw std::invalid_argument("An EventEmitter cannot be its own parent.");
        }
        parent_ = parent;
    }

    // Subscribe a new handler to a specific event ID
    size_t subscribe(const std::string &eventId, HandlerType handler)
    {
        if (eventId.empty())
        {
            throw std::invalid_argument("Event ID cannot be empty.");
        }
        if (!handler)
        {
            throw std::invalid_argument("Handler cannot be null.");
        }
        auto wrappedHandler = std::make_shared<WrappedHandler>(std::move(handler));
        handlers_[eventId].push_back(wrappedHandler);
        return wrappedHandler->getId();
    }

    // Unsubscribe a handler from a specific event ID using the handler ID
    void unsubscribe(const std::string &eventId, size_t handlerId)
    {
        auto it = handlers_.find(eventId);
        if (it == handlers_.end())
        {
            std::cerr << "Attempted to unsubscribe from a non-existent event ID: " << eventId << std::endl;
            return;
        }
        auto &vec = it->second;
        vec.erase(
            std::remove_if(vec.begin(), vec.end(),
                           [handlerId](const std::shared_ptr<WrappedHandler> &h)
                           {
                               return h->getId() == handlerId;
                           }),
            vec.end());

        // Clean up empty handler lists to save memory
        if (vec.empty())
        {
            handlers_.erase(eventId);
        }
    }

    // Notify all subscribed handlers for a specific event ID
    void notify(const std::string &eventId, Args... args)
    {
        if (eventId.empty())
        {
            std::cerr << "Notification with empty event ID ignored." << std::endl;
            return;
        }
        auto it = handlers_.find(eventId);
        if (it != handlers_.end())
        {
            for (const auto &handler : it->second)
            {
                try
                {
                    (*handler)(args..., this);
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Exception during handler invocation: " << e.what() << std::endl;
                }
                catch (...)
                {
                    std::cerr << "Unknown exception during handler invocation." << std::endl;
                }
            }
        }

        // Propagate to the parent if any
        if (parent_)
        {
            parent_->notify(eventId, args...);
        }
    }

private:
    std::unordered_map<std::string, std::vector<std::shared_ptr<WrappedHandler>>> handlers_;
    EventEmitter *parent_ = nullptr;
};