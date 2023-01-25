
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
		}
	private:
		std::set<F*> listeners;
	};
}