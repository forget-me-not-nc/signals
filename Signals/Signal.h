#ifndef SLOT_H
#define SLOT_H

#include <map>
#include <vector>
#include <any>
#include <memory>

#include "SignalHelper.h"

using std::vector;
using std::map;
using std::unique_ptr;

template<typename T>
class Signal
{
protected:
	using Helper = SignalHelper<T>;
	using Callback = typename Helper::Callback;
	using SlotCntxts = std::map<SignalContext<std::any>, std::vector<Callback>, AnyComparator>;

	std::vector<Callback> slots;
	SlotCntxts slotContexts;

public:
	Signal() : slots() {};
	~Signal() = default;

	Signal<T>& addOnce(const Callback& slot, SignalContext<std::any> context = SignalContext<std::any>{}, bool highPriority = false) {
		this->add(
			[this, slot](auto... value) {
				this->remove(slot);
				Helper::invoke(slot, value...); 
			},
			context,
			highPriority
		);

		return *this;
	}

	Signal<T>& add(const Callback& slot, SignalContext<std::any> context = SignalContext<std::any>{}, bool highPriority = false)
	{
		if (highPriority) slots.insert(slots.begin(), slot); 
		else slots.push_back(slot);

		if (context.context.has_value()) slotContexts[context].push_back(slot);

		return *this;
	};

	Signal<T>& remove(const Callback& slot)
	{
		auto it = std::remove_if(slots.begin(), slots.end(), [&slot](const Callback& storedSlot)
			{
				return FunctionComparator<T>{}(storedSlot, slot);
			});

		if (it != slots.end())
		{
			slots.erase(it, slots.end());
		}

		for (auto it = slotContexts.begin(); it != slotContexts.end();)
		{
			auto& callbacks = it->second;

			auto callbackIt = std::remove_if(callbacks.begin(), callbacks.end(), [&slot](const Callback& storedSlot)
				{
					return FunctionComparator<T>{}(storedSlot, slot);
				});

			if (callbackIt != callbacks.end())
			{
				callbacks.erase(callbackIt, callbacks.end());

				if (callbacks.empty()) it = slotContexts.erase(it);
				else ++it;
			}
			else ++it;
		}

		return *this;
	};

	void emit(const T& arg, SignalContext<std::any> context = SignalContext<std::any>{}) {
		if (context.context.has_value())
		{
			auto it = slotContexts.find(context);
			if (it != slotContexts.end())
			{
				for (const auto& callback : it->second)
				{
					Helper::invoke(callback, arg);
				}
			}
		}
		else 
		{
			for (const auto& callback : slots)
			{
				Helper::invoke(callback, arg);
			}
		}
	}
};

#endif;