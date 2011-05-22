#include "../include/gui/Event.hpp"
#include "../include/gui/Defines.hpp"

namespace gui 
{

	SfmlEvent::SfmlEvent( sf::Event* event ) : m_event(event)
	{
		m_type = event ? event->Type : 0;
	}

	WidgetEvent::WidgetEvent( Widget* widget ) : m_widget(widget)
	{

	}

	Widget* WidgetEvent::GetWidget() const
	{
		return m_widget;
	}

	OnClickPressed::OnClickPressed( Widget* widget ) : WidgetEvent(widget)
	{
		m_type = sf::Event::MouseButtonPressed;
	}


	OnClickReleased::OnClickReleased( Widget* widget ): WidgetEvent(widget)
	{
		m_type = sf::Event::MouseButtonReleased;
	}

	OnKeyPressed::OnKeyPressed( Widget* widget, sf::Event::KeyEvent key ) : 
					WidgetEvent(widget), m_key(key)
	{
		m_type = sf::Event::KeyPressed;
	}

	const sf::Event::KeyEvent& OnKeyPressed::GetKey() const
	{
		return m_key;
	}
	OnKeyReleased::OnKeyReleased( Widget* widget,sf::Event::KeyEvent key ) : 
					WidgetEvent(widget), m_key(key)
	{
		m_type = sf::Event::KeyReleased;
	}


	const sf::Event::KeyEvent& OnKeyReleased::GetKey() const
	{
		return m_key;
	}

	OnFocus::OnFocus( Widget* widget ) : WidgetEvent(widget)
	{
		m_type = gui::events::OnFocus;
	}

	OnFocusLost::OnFocusLost( Widget* widget ) : WidgetEvent(widget)
	{
		m_type = gui::events::OnFocusLost;
	}

	OnHover::OnHover( Widget* widget ) : WidgetEvent(widget)
	{
		m_type = gui::events::OnHover;
	}

	OnHoverLost::OnHoverLost( Widget* widget ) : WidgetEvent(widget)
	{
		m_type = gui::events::OnHoverLost;
	}

	OnShow::OnShow( Widget* widget ) : WidgetEvent(widget)
	{
		m_type =  gui::events::OnShow;
	}

	OnHide::OnHide( Widget* widget ) : WidgetEvent(widget)
	{
		m_type = gui::events::OnHide;
	}


	OnResize::OnResize( Widget* widget,const Rect& oldRect ) : 
				WidgetEvent(widget)
	{
		m_type = gui::events::OnResize;
		m_oldRect = oldRect;
	}

	const Rect& OnResize::GetOldRect() const
	{
		return m_oldRect;
	}

	OnMove::OnMove( Widget* widget,const Rect& oldRect ) : 
			WidgetEvent(widget)
	{
		m_type = gui::events::OnMove;
		m_oldRect = oldRect;
	}

	const Rect& OnMove::GetOldRect() const
	{
		return m_oldRect;
	}

	Event::Event() : m_refCount(0),m_type(WIDGET)
	{

	}

	void Event::SetType( uint32 type )
	{
		m_type = type;
	}

	gui::uint32 Event::GetType() const
	{
		return m_type;
	}


	OnDrag::OnDrag( Widget* widget, Drag* drag ): 
					WidgetEvent(widget)
	{
		m_type = events::OnDrag;
		m_drag = drag;
	}

	Drag* OnDrag::GetDrag() const
	{
		return m_drag;
	}

	OnValueChanged::OnValueChanged( Widget* widget, int32 value ) : 
		WidgetEvent(widget), m_value(value)
	{
		m_type = events::OnValueChanged;
	}

	int32 OnValueChanged::GetValue() const
	{
		return m_value;
	}


	OnDoubleClick::OnDoubleClick( Widget* widget /*= NULL*/ ):
			WidgetEvent(widget)
	{
		m_type = events::OnDoubleClick;
	}
}