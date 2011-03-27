#include "../include/gui/DefaultFactory.hpp"

#include "../include/gui/Button.hpp"
#include "../include/gui/CheckBox.hpp"
#include "../include/gui/Label.hpp"
#include "../include/gui/LineEdit.hpp"
#include "../include/gui/RadioBox.hpp"
#include "../include/gui/Slider.hpp"
#include "../include/gui/TextArea.hpp"
#include "../include/gui/Widget.hpp"

namespace gui
{

	DefaultFactory::DefaultFactory()
	{
		
	}

	Widget* DefaultFactory::CreateWidget( uint32 type ) const
	{
		switch (type)
		{
		case WIDGET:	return new Widget;
		case BUTTON:	return new Button;
		case IMAGE_BUTTON: return new ImageButton;
		case LABEL:		return new Label;
		case LINE_EDIT: return new LineEdit;
		case CHECKBOX:	return new CheckBox;
		case RADIOBOX:  return new RadioBox;
		case SLIDER:    return new Slider;
		default: 
			error_log("Unable to create widget of type %u", type);
		}

		return NULL;
	}

	bool DefaultFactory::CanCreateWidget(uint32 type) const
	{
		return type < WIDGETS_COUNT;
	}

	Event* DefaultFactory::CreateEvent( uint32 type ) const
	{
		if(type < sf::Event::Count) {
		}

		return NULL;
	}

	bool DefaultFactory::CanCreateEvent( uint32 type ) const
	{
		return true;
	}
}
