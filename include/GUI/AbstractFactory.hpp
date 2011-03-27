#pragma once

#include "Defines.hpp"
#include "Event.hpp"


namespace gui 
{
	class Widget;
	class AbstractFactory
	{
	public:
		virtual ~AbstractFactory() {}

		//widget creation
		virtual Widget* CreateWidget(uint32 type) const = 0;
		virtual bool CanCreateWidget(uint32 type) const = 0;

		//event and event-commands creation
		virtual Event* CreateEvent(uint32 type) const = 0;
		virtual bool CanCreateEvent(uint32 type) const = 0;
	};

}