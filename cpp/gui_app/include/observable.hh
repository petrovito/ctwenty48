#pragma once

#include <functional>
#include <vector>
namespace c20::gui {

	template<typename T>
	class Observable
	{
		private:
			T obs;
			std::vector<std::function<void(const T&)>> subscribers;
		public:
			void subscribe(const std::function<void(const T&)>& sub)
			{
				subscribers.emplace_back(sub);
			}
			T& get() { return obs; }
			/** Modifies and notifies. */
			void modify(const T& new_obs)
			{
				obs = new_obs;
				notify();
			}
			void notify()
			{
				for (auto& sub: subscribers)
				{
					sub(obs);
				}
			}
	};

}

