#pragma once

#ifndef TFC_CORE_EVENT_INC_H
#define TFC_CORE_EVENT_INC_H

#include <memory>
#include "TFC/Core/Introspect.h"

#ifndef TFC_CORE_H_
#include "TFC/Core.h"
#endif


class TFC::EventClass
{

};

template<typename TBase>
class TFC::EventEmitterClass
{
public:
	template<typename TEventData>
	using Event = TFC::Core::EventObject<TBase*, TEventData>;
};

template<typename TObjectSource, typename TEventData>
class TFC::Core::EventObject
{
public:

	template<typename TEventClass>
	struct EventHandlerTrait {
		typedef void (TEventClass::*Type)(TObjectSource objSource, TEventData eventData);
	};

	typedef void(*EventHandlerInvokerFunc)(void*, TObjectSource, TEventData);

	//typedef typename EventHandlerTrait<EventClass>::Type EventHandler;

	typedef TObjectSource 	SourceType;
	typedef TEventData		EventDataType;

	class EventDelegate;

	EventObject();
	EventObject(bool logDelete);
	~EventObject();
	void operator+=(const EventDelegate& other);
	void operator-=(const EventDelegate& other);
	void operator()(TObjectSource objSource, TEventData eventData) const;

private:
	struct EventNode;
	EventNode* first;

	void* operator new(size_t size) { return ::operator new(size); };

	template<typename, typename>
	friend class TFC::Core::SharedEventObject;

#if VERBOSE
	bool logDelete;
#endif

};

template<typename TObjectSource, typename TEventData>
struct TFC::Core::EventObject<TObjectSource, TEventData>::EventNode
{
	void* instance;
	EventHandlerInvokerFunc eventHandlerInvoker;
	EventNode* next;
};

template<typename TObjectSource, typename TEventData>
class TFC::Core::EventObject<TObjectSource, TEventData>::EventDelegate
{
	void* instance;
	EventHandlerInvokerFunc eventHandlerInvoker;

	EventDelegate(void* instance, EventHandlerInvokerFunc func) : instance(instance), eventHandlerInvoker(func) { }

public:
	template<class TEventClass, typename EventHandlerTrait<TEventClass>::Type funcPtr>
	static EventDelegate PackEventHandler(TEventClass* thisPtr);

	template<typename, typename>
	friend class TFC::Core::EventObject;

	template<typename TEventClass, typename EventHandlerTrait<TEventClass>::Type funcPtr>
	static void EventHandlerInvoker(void*, TObjectSource source, TEventData data);


};

template<typename TObjectSource, typename TEventData>
class TFC::Core::SharedEventObject : protected std::shared_ptr<TFC::Core::EventObject<TObjectSource, TEventData>>
{
public:
	typedef TFC::Core::EventObject<TObjectSource, TEventData> Type;
	SharedEventObject();
	void operator+=(const typename TFC::Core::EventObject<TObjectSource, TEventData>::EventDelegate& other);
	void operator-=(const typename TFC::Core::EventObject<TObjectSource, TEventData>::EventDelegate& other);
	void operator()(TObjectSource objSource, TEventData eventData) const;
};

namespace TFC {
namespace Core {

template<typename TPtrMemFunc, // <- Unfortunately we have to know the type of pointer to member first before actually getting the pointer...
		 TPtrMemFunc ptr,      // <- here
		 typename TIntrospect	 = TFC::Core::Introspect::MemberFunction<TPtrMemFunc>, // <- Instantiate introspector object
		 typename TDeclaringType = typename TIntrospect::DeclaringType,	   // <- Inferring declaring type of pointer to member via introspector
		 typename TObjectSource  = typename TIntrospect::template Args<0>, // <- Inferring TObjectSource by parameter of function pointer
		 typename TEventData 	 = typename TIntrospect::template Args<1>> // <- Inferring TEventData by parameter of function pointer
auto EventHandlerFactory(TDeclaringType* thisPtr)
	-> typename TFC::Core::EventObject<TObjectSource, TEventData>::EventDelegate
{
	return TFC::Core::EventObject<TObjectSource, TEventData>::EventDelegate::template PackEventHandler<TDeclaringType, ptr>(thisPtr);
}

}}


///////////////////////////////////////////////////////////////////////////////////////////////////
// TEMPLATE IMPLEMENTATION
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class TObjectSource, class TEventData>
TFC::Core::EventObject<TObjectSource, TEventData>::EventObject() :
	first(nullptr)
{
#if VERBOSE
	logDelete = false;
#endif
}

template<class TObjectSource, class TEventData>
TFC::Core::EventObject<TObjectSource, TEventData>::EventObject(bool logDelete) :
	first(nullptr)
{
#if VERBOSE
	logDelete = true;
#endif

#if VERBOSE
	if(logDelete)
		dlog_print(DLOG_DEBUG, "TFC-Event", "Event created %d", this);
#endif
}

template<class TObjectSource, class TEventData>
TFC::Core::EventObject<TObjectSource, TEventData>::~EventObject()
{
#if VERBOSE
	if(logDelete)
		dlog_print(DLOG_DEBUG, "TFC-Event", "Event deleted %d", this);
#endif

	auto current = this->first;

	while(current != nullptr)
	{
		auto deleted = current;
		current = current->next;
		delete deleted;
	}

}


template<class TObjectSource, class TEventData>
template<class TEventClass, typename TFC::Core::EventObject<TObjectSource, TEventData>::template EventHandlerTrait<TEventClass>::Type funcPtr>
auto TFC::Core::EventObject<TObjectSource, TEventData>::EventDelegate::PackEventHandler(TEventClass* thisPtr)
	-> TFC::Core::EventObject<TObjectSource, TEventData>::EventDelegate
{
	return { thisPtr, EventHandlerInvoker<TEventClass, funcPtr> };
}


template<class TObjectSource, class TEventData>
void TFC::Core::EventObject<TObjectSource, TEventData>::operator+=(const EventDelegate& other)
{
	auto newNode = new EventNode({ other.instance, other.eventHandlerInvoker, this->first });
	this->first = newNode;
}

template<class TObjectSource, class TEventData>
void TFC::Core::EventObject<TObjectSource, TEventData>::operator-=(const EventDelegate& other)
{
	auto current = this->first;

	while(current != nullptr)
	{
		if(current->instance == other.instance && current->eventHandlerInvoker == other.eventHandlerInvoker)
		{
			//dlog_print(DLOG_DEBUG, "TFC-Event", "Deleting %d %d", current->instance, current->eventHandler);
			if(current == this->first)
				this->first = current->next;

			auto deleted = current;
			current = current->next;
			delete deleted;
		}
		else
		{
			current = current->next;
		}
	}
}

template<typename TObjectSource, typename TEventData>
template<typename TEventClass, typename TFC::Core::EventObject<TObjectSource, TEventData>::template EventHandlerTrait<TEventClass>::Type funcPtr>
void TFC::Core::EventObject<TObjectSource, TEventData>::EventDelegate::EventHandlerInvoker(void* ptr, TObjectSource source, TEventData data)
{
	auto thiz = reinterpret_cast<TEventClass*>(ptr);
	(thiz->*funcPtr)(source, data);
}

template<class TObjectSource, class TEventData>
void TFC::Core::EventObject<TObjectSource, TEventData>::operator() (TObjectSource objSource,
		TEventData eventData) const
{
#if VERBOSE
	if(logDelete)
		dlog_print(DLOG_DEBUG, "TFC-Event", "Event raise %d", this);
#endif

	auto current = this->first;

	while(current != nullptr)
	{
		if(current->instance && current->eventHandlerInvoker)
			current->eventHandlerInvoker(current->instance, objSource, eventData);
		current = current->next;
	}
}


template<typename TObjectSource, typename TEventData>
TFC::Core::SharedEventObject<TObjectSource, TEventData>::SharedEventObject() :
	std::shared_ptr<TFC::Core::EventObject<TObjectSource, TEventData>>(
			new TFC::Core::EventObject<TObjectSource, TEventData>(true))
{

}

template<typename TObjectSource, typename TEventData>
void TFC::Core::SharedEventObject<TObjectSource, TEventData>::operator+=(
		const typename TFC::Core::EventObject<TObjectSource, TEventData>::EventDelegate& other)
{
	// Forwarding the call to the EventObject stored inside this shared_ptr
	this->get()->operator +=(other);
}

template<typename TObjectSource, typename TEventData>
void TFC::Core::SharedEventObject<TObjectSource, TEventData>::operator-=(
		const typename TFC::Core::EventObject<TObjectSource, TEventData>::EventDelegate& other)
{
	// Same as above
	this->get()->operator -=(other);
}

template<typename TObjectSource, typename TEventData>
void TFC::Core::SharedEventObject<TObjectSource, TEventData>:: operator()(
		TObjectSource objSource, TEventData eventData) const
{
	// Same as above
	this->get()->operator ()(objSource, eventData);
}



#define EventHandler(EVENT_METHOD) TFC::Core::EventHandlerFactory<decltype(& EVENT_METHOD), & EVENT_METHOD>(this)
//                                                                ^^^                        ^^^
//                                                                Get the typeof ptr to mbr, then getting the value here
//                                                                This mechanism will be obsolete when C++17 implements auto in template parameter


#endif
