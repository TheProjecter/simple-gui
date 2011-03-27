#pragma once

#include "Defines.hpp"

namespace gui 
{

	class Widget;
	class Event
	{
	public:
		Event();
		virtual ~Event() {}

		uint32 GetType() const;
		void SetType(uint32 type);

		uint32 m_refCount;
		uint32 m_type;
	};

	class SfmlEvent : public Event
	{
	public:
		SfmlEvent(sf::Event* event = NULL);

	protected:
		sf::Event* m_event;
	};

	class WidgetEvent : public Event
	{
	public:
		WidgetEvent(Widget* widget = NULL);

		Widget* GetWidget() const;
	protected:
		Widget* m_widget;
	};

	class OnClickPressed : public WidgetEvent
	{
	public:
		OnClickPressed(Widget* widget = NULL);
	};

	class OnClickReleased : public WidgetEvent
	{
	public:
		OnClickReleased(Widget* widget = NULL);
	};

	class OnKeyPressed : public WidgetEvent
	{
	public:
		OnKeyPressed(Widget* widget = NULL, sf::Event::KeyEvent key = sf::Event::KeyEvent());
		const sf::Event::KeyEvent& GetKey() const;
	protected:
		sf::Event::KeyEvent m_key;
	};

	class OnKeyReleased : public WidgetEvent 
	{
	public:
		OnKeyReleased(Widget* widget, sf::Event::KeyEvent key = sf::Event::KeyEvent());
		const sf::Event::KeyEvent& GetKey() const;
	protected:
		sf::Event::KeyEvent m_key;
	};

	class OnFocus : public WidgetEvent 
	{
	public:
		OnFocus(Widget* widget = NULL);
	};

	class OnFocusLost : public WidgetEvent 
	{
	public:
		OnFocusLost(Widget* widget = NULL);
	};

	class OnHover : public WidgetEvent 
	{
	public:
		OnHover(Widget* widget = NULL);
	};

	class OnHoverLost: public WidgetEvent 
	{
	public: 
		OnHoverLost(Widget* widget = NULL);
	};

	class OnShow : public WidgetEvent
	{
	public:
		OnShow(Widget* widget = NULL);
	};

	class OnHide : public WidgetEvent
	{
	public:
		OnHide(Widget* widget = NULL);
	};

	class OnResize : public WidgetEvent
	{
	public:
		OnResize(Widget* widget = NULL);
	};

	class OnMove : public WidgetEvent
	{
	public:
		OnMove(Widget* widget = NULL);
	};
	
}


