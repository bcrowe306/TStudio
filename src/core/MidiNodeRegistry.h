#ifndef MIDINODEREGISTRY_H
#define MIDINODEREGISTRY_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <uuid.h> // Include the stduuid library
#include <mutex>

namespace tstudio
{
    class MidiNode;

    class MidiNodeRegistry
    {
    public:
        static MidiNodeRegistry &getInstance()
        {
            static MidiNodeRegistry instance;
            return instance;
        }

        void registerNode(std::shared_ptr<MidiNode> node)
        {
            std::lock_guard<std::mutex> lock(mutex);
            nodes.push_back(node);
        }

        void deregisterNode(std::shared_ptr<MidiNode> node)
        {
            std::lock_guard<std::mutex> lock(mutex);
            auto it = std::remove_if(nodes.begin(), nodes.end(),
                                     [&](const std::weak_ptr<MidiNode> &n)
                                     {
                                         return n.lock() == node;
                                     });
            nodes.erase(it, nodes.end());
        }

        void listNodes() const
        {
            std::lock_guard<std::mutex> lock(mutex);
            std::cout << "Registered MidiNodes:" << std::endl;
            for (const auto &weakNode : nodes)
            {
                if (auto node = weakNode.lock())
                {
                    // std::cout << "Node " << node->getName() << " (" << uuids::to_string(node->getId()) << ")" << std::endl;
                }
            }
        }

    private:
        MidiNodeRegistry() = default;
        ~MidiNodeRegistry() = default;
        MidiNodeRegistry(const MidiNodeRegistry &) = delete;
        MidiNodeRegistry &operator=(const MidiNodeRegistry &) = delete;

        mutable std::mutex mutex;
        std::vector<std::weak_ptr<MidiNode>> nodes;
    };
};

#endif // !MIDINODEREGISTRY_H




