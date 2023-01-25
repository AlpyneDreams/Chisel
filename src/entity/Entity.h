#pragma once

#include <concepts>
#include <string>

#include "common/Reflection.h"
#include "common/Traits.h"

#include "Common.h"
#include "Scene.h"
#include "Component.h"

#include "components/Name.h"

#include "rain/rain.h"

namespace engine
{
    /**
     * Entity is a handle for a scene entity.
     *   - It consists of a scene registry and an ID.
     *   - It is a container for components, which
     *     provide both logic and data.
     */
    struct Entity : rain::Reflect
    {
        using SelectionID = std::underlying_type_t<EntityID>;
        
        // Handle: registry ptr + entity ID
        Handle handle;

        // Create a new entity
        Entity() : handle(World.CreateEntity()) {}

        // Grab an entity by handle or scene & ID
        Entity(Handle&& handle) : handle(handle) {}
        Entity(Scene& scene, EntityID id) : handle(scene.ents, id) {}

        // If no scene is specified use World
        Entity(EntityID id) : handle(World.ents, id) {}
        Entity(SelectionID id) : handle(World.ents, EntityID(id)) {}

        operator Handle() const { return handle; }
        operator EntityID() const { return handle.entity(); }
        operator SelectionID() const { return SelectionID(handle.entity()); }

        operator bool() const { return handle.entity() != EntityNull && handle.valid(); }

        bool operator==(const Entity& that) const { return handle == that.handle; }
        bool operator==(const EntityID& that) const { return handle == that; }
        
        Scene& GetScene() const {
            return *handle.registry()->ctx().at<Scene*>();
        }

        // Delete an entity, (handle will become null)
        void Delete() {
            handle.registry()->destroy(handle.entity());
            handle = Handle(World.ents, EntityNull);
        }

    // Name Component //

        void SetName(const char* name)
        {
            if (name != nullptr) {
                GetOrAddComponent<Name>().name = name;
            } else {
                RemoveComponent<Name>();
            }
        }

        void SetName(std::string& name)
        {
            if (!name.empty()) {
                GetOrAddComponent<Name>().name = name;
            } else {
                RemoveComponent<Name>();
            }
        }

        const char* GetName() const
        {
            if (HasComponent<Name>())
            {
                std::string& name = GetComponent<Name>().name;
                if (!name.empty())
                    return name.c_str();
            }

            return nullptr;
        }

    // Component Management //

        template <class C>
        bool HasComponent() const {
            return handle.any_of<C>();
        }

        template <class C>
        C& AddComponent()
        {
            // Add required components, if any
            if constexpr (derived_from_template<C, RequireComponents>) {
                C::AddRequiredComponents(handle);
            }

            // Create component instance (get if it already exists)
            C& component = handle.get_or_emplace<C>();
            
            // If this component is a behavior, then share our handle with it
            if constexpr (std::derived_from<C, Behavior>) {
                Attach(static_cast<Behavior&>(component));
            }
            
            return component;
        }

        template <class C>
        C& GetComponent() const {
            return handle.get<C>();
        }
        
        // TODO: Potentially use custom iterators that check storage has entity on the fly
        using ComponentList = std::vector<std::pair<ComponentID, Component*>>;
    
        // TODO: Get components with base class (this version would be for T = Component)
        ComponentList GetComponents() const
        {
            ComponentList components;
            for (auto&& [id, storage] : handle.registry()->storage())
            {
                if (!storage.contains(handle.entity()))
                    continue;
                
                components.emplace_back(id, (Component*)storage.get(handle.entity()));
            }
            return components;
        }

        template <class C>
        C& GetOrAddComponent()
        {
            if (!HasComponent<C>()) {
                return AddComponent<C>();
            }
            return GetComponent<C>();
        }

        template <class C>
        void RemoveComponent()
        {
            if constexpr (std::derived_from<C, Behavior>) {
                if (HasComponent<C>())
                    Detach(static_cast<Behavior&>(GetComponent<C>()));
            }
            // remove<C>(): the component does not need to exist
            handle.remove<C>();
        }

    // Low-Level Component Management //

        entt::sparse_set& GetStorage(ComponentID id)
        {
            auto registry   = handle.registry();
            auto storage    = registry->storage(id);
            auto end        = registry->storage().cend();
            if (storage == end) [[unlikely]] // TODO: Runtime component storage
                return registry->storage<entt::any>(id); // This will not work.
            return storage->second;
        }

        bool HasComponent(ComponentID id)
        {
            return GetStorage(id).contains(handle.entity());
        }

        Component* GetComponent(ComponentID id)
        {
            return (Component*)GetStorage(id).get(handle.entity());
        }

        // TODO: This should behave 1:1 with AddComponent.
        // Perhaps there should be only one implementation of this logic.
        void* AddComponent(ComponentID id)
        {
            if (HasComponent(id))
                return GetComponent(id);
            
            // TODO: Add required components
            
            GetStorage(id).emplace(handle.entity());

            Component* component = GetComponent(id);

            // If this component is a behavior, then share our handle with it
            if (rain::Class::Get(id)->DerivedFrom<Behavior>()) {
                Attach(*(Behavior*)component);
            }
            
            return component;
        }

        void RemoveComponent(ComponentID id) {
            handle.registry()->storage(id)->second.remove(handle.entity());
        }

    // Entity Clone //

        Entity Clone()
        {
            Entity clone = World.CreateEntity();
            
            // Clone components
            for (auto&& [id, storage] : handle.registry()->storage())
            {
                if (!storage.contains(handle.entity()))
                    continue;

                storage.emplace(clone, storage.get(handle));
            }
            
            return clone;
        }

    private:
        // Implemented in Behavior.h
        void Attach(Behavior& behavior);
        void Detach(Behavior& behavior);
    };

    // A pure Entity cannot be a component.
    template<> Entity& Entity::AddComponent<Entity>() = delete;
    template<> Entity& Entity::GetComponent<Entity>() const = delete;
    template<> void Entity::RemoveComponent<Entity>() = delete;
}