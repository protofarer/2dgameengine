#pragma once

#include "../Logger/Logger.h"
#include "Event.h"
#include <map>
#include <typeindex>
#include <list>
#include <memory>
#include <functional>

// as done before for this trick, virtual functions
class IEventCallback {
	private:
		virtual void Call(Event& e) = 0;
	public:
		virtual ~IEventCallback() = default;

		void Execute(Event& e) {
			Call(e);
		}
};

// wrapper around function ptr
template <typename TOwner, typename TEvent>
class EventCallback: public IEventCallback {
	private:
		// since event callback functions needs to keep track of function ptr
		typedef void (TOwner::*CallbackFunction)(TEvent&);

		TOwner* ownerInstance;
		CallbackFunction callbackFunction;

		virtual void Call(Event& e) override {
			// * invoke a function ptr
			std::invoke(callbackFunction, ownerInstance, static_cast<TEvent&>(e));
		}

	public:
		EventCallback(TOwner* ownerInstance, CallbackFunction callbackFunction): 
			ownerInstance(ownerInstance), callbackFunction(callbackFunction) {};

		virtual ~EventCallback() override = default;
};

typedef std::list<std::unique_ptr<IEventCallback>> HandlerList;

class EventBus {
	private:
		std::map<std::type_index, std::unique_ptr<HandlerList>> subscribers;
	public:
		EventBus() {
			Logger::Log("EventBus constructor called");
		}
		~EventBus() {
			Logger::Log("EventBus constructor called");	
		}

		// Clear subscribers list
		void Reset() {
			subscribers.clear();
		}

		//////////////////////////////////////////////////
		// Subscribe to an event type <T>
		// Listeners subscribe to events
		// Eg: eventBus -> SubscribeToEvent<CollisionEvent>(this, &Game::onCollision);
		//////////////////////////////////////////////////
		template <typename TEvent, typename TOwner>
		void SubscribeToEvent(TOwner* ownerInstance, void (TOwner::*callbackFunction)(TEvent&)) {
			if (!subscribers[typeid(TEvent)].get()) {
				// handle nullptr
				subscribers[typeid(TEvent)] = std::make_unique<HandlerList>();
			}
			auto subscriber = std::make_unique<EventCallback<TOwner, TEvent>>(ownerInstance, callbackFunction);
			subscribers[typeid(TEvent)]->push_back(std::move(subscriber));
		}
		//////////////////////////////////////////////////
		// Emit event of type <T>
		// Upon emit, execute listener callbacks
		// Eg: eventBus -> EmitEvent<CollisionEvent>(player, enemy);
		//////////////////////////////////////////////////
		template <typename TEvent, typename ...TArgs>
		void EmitEvent(TArgs&& ...args) {
			auto handlers = subscribers[typeid(TEvent)].get();
			if (handlers) {
				for (auto it = handlers->begin(); it != handlers->end(); it++) {
					auto handler = it->get();
					TEvent event(std::forward<TArgs>(args)...);
					handler->Execute(event);
				}
			}
		}
};