#pragma once

#include "AbstractFactory.hpp"
#include "Widget.hpp"

namespace gui 
{

	class DefaultFactory : public AbstractFactory
	{
	public:
		DefaultFactory();
		Widget* CreateWidget(uint32 type) const;
		bool CanCreateWidget(uint32 type) const;

		Event* CreateEvent(uint32 type) const;
		bool CanCreateEvent(uint32 type) const;
	};

}
