#pragma once
#include <functional>
#include <set>

namespace chisel
{
    // TODO: std::function...
    template <typename... Args>
	class Event
	{
		using F = void(Args...);
	public:
		Event& operator +=(F* func) {
			listeners.insert(func);
			return *this;
		}

		Event& operator -=(F* func) {
			listeners.erase(func);
			return *this;
		}

		void operator ()(Args... args) {
			for (auto* func : listeners) {
				func(args...);
			}
			for (auto* func : once) {
				func(args...);
			}
			once.clear();
		}

		void Once(F* func) {
			once.insert(func);
		}

	private:
		std::set<F*> listeners;
		std::set<F*> once;
	};
}