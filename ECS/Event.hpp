#pragma once

#include <unordered_set>
#include <concepts>

namespace ECS
{
	template<typename... Args>
	class Event
	{
	private:
		struct BaseHandler
		{
			virtual void Invoke(Args... args) = 0;
		};

		template<std::invocable<Args...> T>
		struct Handler : public BaseHandler
		{
			Handler(T invokable) : Invokable(invokable) {}

			const T Invokable;

			virtual void Invoke(Args... args) override { Invokable(args...); }
		};

		std::vector<BaseHandler*> m_eventHandlers;
	public:
		Event() {}

		template<std::invocable<Args...> T>
		void operator+=(const T& handler)
		{
			m_eventHandlers.push_back(new Handler<T>(handler));
		}

		~Event()
		{
			for(auto handler : m_eventHandlers)
			{
				delete handler;
			}
		}

		void operator()(Args... args)
		{
			for(auto handler : m_eventHandlers)
			{
				handler->Invoke(args...);
			}
		}
	};

	class Entity;

	template<typename... Args>
	class EntityEvent
	{
	private:
		static size_t s_nextId;
	public:
		EntityEvent() : Id(s_nextId++) {}

		const size_t Id;

		void operator()(Entity& entity, Args... args);
	};

	template<typename... Args>
	size_t EntityEvent<Args...>::s_nextId(0);
}