#pragma once
#include <unordered_map>

template<class Resource>
class ResourceList
{
public:
    void add(const std::string& name, Resource resource) {
        resources[name] = resource;
    }

    Resource get(const std::string& name) {
        return resources[name];
    }

    bool has(const std::string& name) const {
        return resources.find(name) != resources.end();
    };

    void reserve(size_t newSize) {
        resources.reserve(newSize);
    }

    void destroy() {
        for (auto& pair : resources) {
            bgfx::destroy(pair.second);
        }
    }

private:
    std::unordered_map<std::string, Resource> resources;
};
