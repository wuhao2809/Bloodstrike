#pragma once
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <memory>
#include <cassert>

using EntityID = std::size_t;

class ComponentPool {
public:
    virtual ~ComponentPool() = default;
    virtual void remove(EntityID entity) = 0;
};

template<typename T>
class ComponentPoolTyped : public ComponentPool {
private:
    std::unordered_map<EntityID, T> components;

public:
    void add(EntityID entity, T component) {
        components[entity] = std::move(component);
    }

    T* get(EntityID entity) {
        auto it = components.find(entity);
        return it != components.end() ? &it->second : nullptr;
    }

    void remove(EntityID entity) override {
        components.erase(entity);
    }

    std::unordered_map<EntityID, T>& getAll() {
        return components;
    }
};

class ECS {
private:
    EntityID nextEntityID = 1;
    std::unordered_map<std::type_index, std::unique_ptr<ComponentPool>> componentPools;

public:
    EntityID createEntity() {
        return nextEntityID++;
    }

    template<typename T>
    void addComponent(EntityID entity, T component) {
        auto typeIndex = std::type_index(typeid(T));
        
        if (componentPools.find(typeIndex) == componentPools.end()) {
            componentPools[typeIndex] = std::make_unique<ComponentPoolTyped<T>>();
        }
        
        auto pool = static_cast<ComponentPoolTyped<T>*>(componentPools[typeIndex].get());
        pool->add(entity, std::move(component));
    }

    template<typename T>
    T* getComponent(EntityID entity) {
        auto typeIndex = std::type_index(typeid(T));
        auto it = componentPools.find(typeIndex);
        
        if (it == componentPools.end()) {
            return nullptr;
        }
        
        auto pool = static_cast<ComponentPoolTyped<T>*>(it->second.get());
        return pool->get(entity);
    }

    template<typename T>
    std::unordered_map<EntityID, T>& getComponents() {
        auto typeIndex = std::type_index(typeid(T));
        auto it = componentPools.find(typeIndex);
        
        if (it == componentPools.end()) {
            componentPools[typeIndex] = std::make_unique<ComponentPoolTyped<T>>();
        }
        
        auto pool = static_cast<ComponentPoolTyped<T>*>(componentPools[typeIndex].get());
        return pool->getAll();
    }

    void removeEntity(EntityID entity) {
        for (auto& [typeIndex, pool] : componentPools) {
            pool->remove(entity);
        }
    }
};
