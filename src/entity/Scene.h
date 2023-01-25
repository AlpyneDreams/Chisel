#pragma once

#include "Common.h"
#include "engine/System.h"

namespace engine
{
    // The global scene that contains all other scenes.
    extern inline struct Scene World;

    /** 
     *  A scene is a collection of entities with  
     *  components assigned to them. All active scenes
     *  are children of World.
     */
    struct Scene : SystemGroup
    {
        // TODO: Handle integrating subscenes into the parent.
        Scene* parent = &World;
    private:
        entt::registry ents;
    public:

        Scene(Scene* parent = &World) : parent(parent) {
            ents.ctx().emplace<Scene*>(this);
        }

        Handle CreateEntity() {
            return Handle(ents, ents.create());
        }

        void DeleteEntity(EntityID id) {
            ents.destroy(id);
        }


        // TODO: Custom view class that wraps basic_view and is iterable?
        template <typename... Components>
        auto Each() {
            return ents.view<Components...>().each();
        }

        // Run a function for each entity
        void Each(auto function) const {
            ents.each(function);
        }
    private:
        friend struct Entity;
    };

    // The global scene that contains all other scenes.
    inline Scene World = Scene(nullptr);
}