#pragma once
#include <functional>
#include <vector>

namespace chisel
{
    template <typename... Args>
	class Event
	{
		using F = void(Args...);
	public:
		Event& operator +=(auto func) {
			listeners.push_back(func);
			return *this;
		}

		Event& operator -=(auto func) {
			listeners.erase(std::remove(listeners.begin(), listeners.end(), func), listeners.end());
			return *this;
		}

		void operator ()(Args... args) {
			for (auto& func : listeners) {
				func(args...);
			}
			for (auto& func : once) {
				func(args...);
			}
			once.clear();
		}

		void Once(auto func) {
			once.push_back(func);
		}

	private:
		std::vector<std::function<F>> listeners;
		std::vector<std::function<F>> once;
	};
}