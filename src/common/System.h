#pragma once

#include <list>
#include <memory>
#include <map>
#include <typeinfo>
#include <typeindex>
#include <type_traits>

#include "common/Ranges.h"

namespace chisel
{
    struct System
    {
        virtual void Start() {}
        virtual void Update() {}
        virtual void Tick() {}

        virtual ~System() {}
    };

    using SystemFunc = void(System*);

    template <class T>
    concept SystemClass = std::is_base_of_v<System, T>;

    struct SystemGroup : System
    {
    private:
        bool started = false;

        // TODO: non-owned systems?
        // TODO: let systems define their own update order?
        //       (currently using insertion order)
        // TODO: store and retrieve systems in update order?

        struct Callback {
            System* system;
            SystemFunc* func;
        };

        std::list<Callback> OnStart, OnUpdate, OnTick;


        struct SystemRecord {
            std::shared_ptr<System> system;
            std::list<Callback>::iterator Start, Update, Tick;
        };

        std::multimap<std::type_index, SystemRecord> systems;

    public:
        // Can be destructured to a std::pair<std::type_index, std::shared_ptr<System>>
        using Iterator   = decltype(systems)::const_iterator;
        using SystemList = Subrange<Iterator>;

        SystemGroup() {}
        SystemGroup(auto*... sys) {
            (systems.insert({typeid(decltype(sys)), {std::shared_ptr<System>(sys)}}), ...);
        }

        template <SystemClass Sys>
        Sys& AddSystem(auto&... args)
        {
            static System nullsystem;

            auto system = std::make_shared<Sys>(args...);
            auto* sys   = system.get();

            auto& [id, record] = *systems.insert({typeid(Sys), {system}});
            record.Start  = OnStart.end();
            record.Update = OnUpdate.end();
            record.Tick   = OnTick.end();

            record.Start = RegisterCallback(OnStart, sys, [](System* sys) { sys->Start(); });

            // If Start() has already been called, then call
            // it on new systems as soon as they're created.
            if (started) {
                system->Start();
            }

            record.Update = RegisterCallback(OnUpdate, sys, [](System* sys) { sys->Update(); });

            record.Tick = RegisterCallback(OnTick, sys, [](System* sys) { sys->Tick(); });

            return *sys;
        }

        template <SystemClass Sys>
        SystemList GetSystems() const {
            return GetSystems(typeid(Sys));
        }

        SystemList GetSystems(std::type_index type) const {
            return systems.equal_range(type);
        }

        // Remove all systems of type Sys
        template <SystemClass Sys>
        void RemoveSystems() {
            RemoveSystems(typeid(Sys));
        }

        // Remove all systems of 'type'
        void RemoveSystems(std::type_index type) {
            for (auto& [t, record] : GetSystems(type)) {
                UnregisterCallbacks(record);
            }
            systems.erase(type);
        }

        // Removes a single system
        void RemoveSystem(Iterator iterator) {
            UnregisterCallbacks(iterator->second);
            systems.erase(iterator);
        }

        // Removes a single system
        void RemoveSystem(System* system) {
            for (auto it = systems.begin(); it != systems.end(); ++it) {
                if (it->second.system.get() == system) {
                    UnregisterCallbacks(it->second);
                    systems.erase(it);
                    return;
                }
            }
        }

    // Iteration:

        Iterator begin() const {return systems.begin();}
        Iterator end() const {return systems.end();}

    // System overrides:

        void Start() final override { started = true; Call(OnStart); }
        void Update() final override { Call(OnUpdate); }
        void Tick() final override { Call(OnTick); }

    protected:

        inline void Call(auto& event) {
            int i = 0;
            for (auto& [sys, Func] : event) {
                // TODO: Why is this necessary?
                if (i++ > event.size()) return;
                Func(sys);
            }
        }

        inline auto RegisterCallback(auto& event, System* system, SystemFunc* func) {
            return event.insert(event.end(), Callback {system, func});
        }

        inline void UnregisterCallbacks(const SystemRecord& record)
        {
            if (record.Start != OnStart.end())  OnStart.erase(record.Start);
            if (record.Update != OnUpdate.end()) OnUpdate.erase(record.Update);
            if (record.Tick != OnTick.end())   OnTick.erase(record.Tick);
        }
    };
}