#pragma once
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <memory>
#include <cassert>
#include <utility>
#include <cstddef>
#include <algorithm>

using EntityID = std::size_t;

// -------------------------------
// Component pool base interface
// -------------------------------
class ComponentPool {
public:
    virtual ~ComponentPool() = default;

    // Remove a component for an entity (no-op if absent)
    virtual void remove(EntityID entity) = 0;

    // Presence / stats
    virtual bool has(EntityID entity) const = 0;
    virtual std::size_t size() const = 0;

    // Maintenance
    virtual void clear() = 0;
    virtual void shrink_to_fit() = 0;

    // Clone component from -> to (returns true if source existed)
    virtual bool cloneTo(EntityID from, EntityID to) = 0;
};

// --------------------------------------
// Packed pool (dense storage) per type T
// --------------------------------------
template<typename T>
class ComponentPoolTyped : public ComponentPool {
private:
    // Dense parallel arrays: entities <-> data
    std::vector<EntityID> entities_;
    std::vector<T>        data_;
    // Sparse map: entity -> dense index
    std::unordered_map<EntityID, std::size_t> index_;

    void swapErase(std::size_t idx) {
        const std::size_t last = data_.size() - 1;
        if (idx != last) {
            // Move last into idx
            data_[idx] = std::move(data_[last]);
            EntityID movedEntity = entities_[last];
            entities_[idx] = movedEntity;
            index_[movedEntity] = idx;
        }
        data_.pop_back();
        entities_.pop_back();
    }

public:
    // Add or replace component
    void add(EntityID entity, T component) {
        auto it = index_.find(entity);
        if (it != index_.end()) {
            data_[it->second] = std::move(component);
        } else {
            index_[entity] = data_.size();
            entities_.push_back(entity);
            data_.push_back(std::move(component));
        }
    }

    // In-place construct
    template<class... Args>
    T& emplace(EntityID entity, Args&&... args) {
        auto it = index_.find(entity);
        if (it != index_.end()) {
            data_[it->second] = T(std::forward<Args>(args)...);
            return data_[it->second];
        } else {
            index_[entity] = data_.size();
            entities_.push_back(entity);
            data_.emplace_back(std::forward<Args>(args)...);
            return data_.back();
        }
    }

    // Get pointer (nullptr if absent)
    T* get(EntityID entity) {
        auto it = index_.find(entity);
        return it != index_.end() ? &data_[it->second] : nullptr;
    }
    const T* get(EntityID entity) const {
        auto it = index_.find(entity);
        return it != index_.end() ? &data_[it->second] : nullptr;
    }

    // Get reference (asserts presence)
    T& getRef(EntityID entity) {
        auto it = index_.find(entity);
        assert(it != index_.end());
        return data_[it->second];
    }
    const T& getRef(EntityID entity) const {
        auto it = index_.find(entity);
        assert(it != index_.end());
        return data_[it->second];
    }

    // ComponentPool overrides
    void remove(EntityID entity) override {
        auto it = index_.find(entity);
        if (it == index_.end()) return;
        const std::size_t idx = it->second;
        index_.erase(it);
        swapErase(idx);
    }

    bool has(EntityID entity) const override {
        return index_.find(entity) != index_.end();
    }

    std::size_t size() const override { return data_.size(); }

    void clear() override {
        entities_.clear();
        data_.clear();
        index_.clear();
    }

    void shrink_to_fit() override {
        entities_.shrink_to_fit();
        data_.shrink_to_fit();
        index_.rehash(0);
    }

    bool cloneTo(EntityID from, EntityID to) override {
        auto it = index_.find(from);
        if (it == index_.end()) return false;
        add(to, data_[it->second]);
        return true;
    }

    // Dense accessors (useful for tight iteration)
    const std::vector<EntityID>& entities() const { return entities_; }
    std::vector<T>& data() { return data_; }
    const std::vector<T>& data() const { return data_; }

    // Reserve capacity
    void reserve(std::size_t n) {
        entities_.reserve(n);
        data_.reserve(n);
        index_.reserve(n);
    }

    // Iterate all (EntityID, T&) pairs
    template<typename F>
    void each(F&& f) {
        for (std::size_t i = 0; i < data_.size(); ++i) {
            f(entities_[i], data_[i]);
        }
    }
};

// -------------------------------
// ECS container
// -------------------------------
class ECS {
private:
    EntityID nextEntityID_ = 1;
    std::vector<EntityID> freeList_;
    std::vector<uint8_t>  alive_; // index by EntityID, 1 = alive
    std::unordered_map<std::type_index, std::unique_ptr<ComponentPool>> componentPools_;

    template<typename T>
    ComponentPoolTyped<T>* ensurePool() {
        const std::type_index ti(typeid(T));
        auto it = componentPools_.find(ti);
        if (it == componentPools_.end()) {
            auto up = std::make_unique<ComponentPoolTyped<T>>();
            auto ptr = up.get();
            componentPools_.emplace(ti, std::move(up));
            return ptr;
        }
        return static_cast<ComponentPoolTyped<T>*>(it->second.get());
    }

    void ensureAliveSize(EntityID id) {
        if (id >= alive_.size()) alive_.resize(id + 1, 0);
    }

public:
    // --------- Entities ----------
    EntityID createEntity() {
        EntityID id;
        if (!freeList_.empty()) {
            id = freeList_.back();
            freeList_.pop_back();
        } else {
            id = nextEntityID_++;
            ensureAliveSize(id);
        }
        ensureAliveSize(id);
        alive_[id] = 1;
        return id;
    }

    bool isAlive(EntityID id) const {
        return id < alive_.size() && alive_[id] != 0;
    }

    void removeEntity(EntityID entity) {
        if (!isAlive(entity)) return;
        // remove from all pools
        for (auto& kv : componentPools_) {
            kv.second->remove(entity);
        }
        alive_[entity] = 0;
        freeList_.push_back(entity);
    }

    // --------- Components ----------
    template<typename T>
    void addComponent(EntityID entity, T component) {
        assert(isAlive(entity) && "addComponent on dead entity");
        ensurePool<T>()->add(entity, std::move(component));
    }

    template<typename T, class... Args>
    T& emplaceComponent(EntityID entity, Args&&... args) {
        assert(isAlive(entity) && "emplaceComponent on dead entity");
        return ensurePool<T>()->emplace(entity, std::forward<Args>(args)...);
    }

    template<typename T>
    bool hasComponent(EntityID entity) const {
        const auto it = componentPools_.find(std::type_index(typeid(T)));
        if (it == componentPools_.end()) return false;
        return static_cast<ComponentPoolTyped<T>*>(it->second.get())->has(entity);
    }

    template<typename T>
    T* getComponent(EntityID entity) {
        const auto it = componentPools_.find(std::type_index(typeid(T)));
        if (it == componentPools_.end()) return nullptr;
        return static_cast<ComponentPoolTyped<T>*>(it->second.get())->get(entity);
    }

    template<typename T>
    const T* getComponent(EntityID entity) const {
        const auto it = componentPools_.find(std::type_index(typeid(T)));
        if (it == componentPools_.end()) return nullptr;
        return static_cast<const ComponentPoolTyped<T>*>(it->second.get())->get(entity);
    }

    template<typename T>
    void removeComponent(EntityID entity) {
        const auto it = componentPools_.find(std::type_index(typeid(T)));
        if (it == componentPools_.end()) return;
        static_cast<ComponentPoolTyped<T>*>(it->second.get())->remove(entity);
    }

    template<typename T>
    std::size_t componentCount() const {
        const auto it = componentPools_.find(std::type_index(typeid(T)));
        if (it == componentPools_.end()) return 0;
        return it->second->size();
    }

    template<typename T>
    void clearComponents() {
        ensurePool<T>()->clear();
    }

    template<typename T>
    void shrinkComponentsToFit() {
        ensurePool<T>()->shrink_to_fit();
    }

    template<typename T>
    void reserveComponents(std::size_t n) {
        ensurePool<T>()->reserve(n);
    }

    // Dense views (entities + data vectors)
    template<typename T>
    const std::vector<EntityID>& denseEntities() const {
        return static_cast<const ComponentPoolTyped<T>*>(componentPools_.at(std::type_index(typeid(T))).get())->entities();
    }

    template<typename T>
    std::vector<T>& denseData() {
        return ensurePool<T>()->data();
    }
    template<typename T>
    const std::vector<T>& denseData() const {
        return static_cast<const ComponentPoolTyped<T>*>(componentPools_.at(std::type_index(typeid(T))).get())->data();
    }

    // Clone all components from one entity to another (existing components on 'to' are replaced)
    void cloneEntityComponents(EntityID from, EntityID to) {
        assert(isAlive(to) && "cloneEntityComponents: target not alive");
        for (auto& kv : componentPools_) {
            kv.second->cloneTo(from, to);
        }
    }

    // --------- Queries / iteration ----------
    // forEach over entities having all specified component types.
    // Performance tip: put the rarest component first.
    template<typename T0, typename... Ts, typename Func>
    void forEach(Func&& f) {
        auto* base = ensurePool<T0>();
        const auto& ents = base->entities();
        for (std::size_t i = 0; i < ents.size(); ++i) {
            EntityID e = ents[i];
            if constexpr (sizeof...(Ts) > 0) {
                if (!(hasComponent<Ts>(e) && ...)) continue;
                f(e, base->data()[i], *getComponent<Ts>(e)...);
            } else {
                f(e, base->data()[i]);
            }
        }
    }

    // Const version
    template<typename T0, typename... Ts, typename Func>
    void forEach(Func&& f) const {
        const auto it0 = componentPools_.find(std::type_index(typeid(T0)));
        if (it0 == componentPools_.end()) return;
        const auto* base = static_cast<const ComponentPoolTyped<T0>*>(it0->second.get());
        const auto& ents = base->entities();
        const auto& data = base->data();
        for (std::size_t i = 0; i < ents.size(); ++i) {
            EntityID e = ents[i];
            if constexpr (sizeof...(Ts) > 0) {
                if (!(hasComponent<Ts>(e) && ...)) continue;
                f(e, data[i], *getComponent<Ts>(e)...);
            } else {
                f(e, data[i]);
            }
        }
    }
};
