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
		SfmlEvent(sf::Event* event);

	protected:
		sf::Event* m_event;
	};

	class WidgetEvent : public Event
	{
	public:
		WidgetEvent(Widget* widget);

		Widget* GetWidget() const;
	protected:
		Widget* m_widget;
	};

	class OnClickPressed : public WidgetEvent
	{
	public:
		OnClickPressed(Widget* widget);
	};

	class OnClickReleased : public WidgetEvent
	{
	public:
		OnClickReleased(Widget* widget);
	};

	class OnDoubleClick : public WidgetEvent
	{
	public:
		OnDoubleClick(Widget* widget);
	};

	class OnKeyPressed : public WidgetEvent
	{
	public:
		OnKeyPressed(Widget* widget, sf::Event::KeyEvent key);
		const sf::Event::KeyEvent& GetKey() const;
	protected:
		sf::Event::KeyEvent m_key;
	};

	class OnKeyReleased : public WidgetEvent 
	{
	public:
		OnKeyReleased(Widget* widget, sf::Event::KeyEvent key);
		const sf::Event::KeyEvent& GetKey() const;
	protected:
		sf::Event::KeyEvent m_key;
	};

	class OnFocus : public WidgetEvent 
	{
	public:
		OnFocus(Widget* widget);
	};

	class OnFocusLost : public WidgetEvent 
	{
	public:
		OnFocusLost(Widget* widget);
	};

	class OnHover : public WidgetEvent 
	{
	public:
		OnHover(Widget* widget);
	};

	class OnHoverLost: public WidgetEvent 
	{
	public: 
		OnHoverLost(Widget* widget);
	};

	class OnShow : public WidgetEvent
	{
	public:
		OnShow(Widget* widget);
	};

	class OnHide : public WidgetEvent
	{
	public:
		OnHide(Widget* widget);
	};

	class OnResize : public WidgetEvent
	{
	public:
		OnResize(Widget* widget,const Rect& oldRect);
		const Rect& GetOldRect() const;
	private:
		Rect m_oldRect;
	};

	class OnMove : public WidgetEvent
	{
	public:
		OnMove(Widget* widget,const Rect& oldRect);
		const Rect& GetOldRect() const;
	private:
		Rect m_oldRect;
	};
	class OnValueChanged : public WidgetEvent
	{
	public:
		OnValueChanged(Widget* widget, int32 value);
		
		int32 GetValue() const;
	private:
		int32 m_value;
	};

	class Drag;
	class OnDrag : public WidgetEvent
	{
	public:
		OnDrag(Widget* widget, Drag* drag);
		Drag* GetDrag() const;
	private:
		Drag* m_drag;
	};
	
}


