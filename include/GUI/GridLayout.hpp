#pragma once

#include "Defines.hpp"
#include "Widget.hpp"

namespace gui
{

	class LayoutItem {
	public:

		//     upper part
		//  ______\/______
		//l |_|________|_| } 5px panning
		//e | |        | | 
		//f=|>| proper |<|== right part
		//t |_|________|_|
		//  |______^_____|
		//     lower part
	
		enum CollisionType {
			NoCollision,		//it didn't collide
			ProperCollision,	//collided with the widget area
			UpperCollision,		//collided with the upper part of the rect
			LowerCollision,		//collided with the lower part of the rect
			LeftCollision,		//collided with the left part of the rect
			RightCollision		//collided with the right part of the rect
		};
		LayoutItem(Widget* widget = NULL, bool expanded = false);
		CollisionType IsCollision(int x, int y) const;
		void SetSize(const Rect& rect);
		const Rect& GetSize() const;

		void SetWidget(Widget* widget);
		Widget* GetWidget() const;

		bool empty() const;

		void ResizeWidget(uint32 panning);
	private:
		Rect m_rect;
		sf::Shape m_shape;
		Widget* m_widget;
		bool m_expanded;
	};

	class GridLayout : public Widget
	{
	public:
		GridLayout();
		~GridLayout();

		void AddWidgetToGrid(Widget* widget, uint32 line, uint32 column);
		Widget* RemoveWidgetAt(uint32 line, uint32 column);

		void AddLineBefore(uint32 line);
		void AddLineAfter(uint32 line);

		void AddColumnBefore(uint32 column);
		void AddColumnAfter(uint32 column);

		void HandleDragDrop(Drag* drag);
		
		void SetPanning(uint32 panning);
		uint32 GetPanning() const;
	private:
		void ComputeCells();					//internally used to resize the cells
		void AddWidget(Widget* child);			//only internally used
		bool RemoveWidget(Widget* widget);		//internally used as well

		//lines		//columns
		std::vector<std::vector<LayoutItem> > m_items;
		uint32 m_panning;						//panning for widgets
	};


}