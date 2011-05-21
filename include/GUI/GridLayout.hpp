#pragma once

#include "Defines.hpp"
#include "Widget.hpp"

namespace gui
{

	class LayoutItem {
	public:

		//     upper part
		//  ______\/______
		//l |_|________|_| }panning
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
		CollisionType IsCollision(int x, int y, uint32 panning) const;

		void SetPos(int32 x, int32 y);
		void SetSize(uint32 width, uint32 height, bool extremity);

		void SetRect(int32 x, int32 y, uint32 w, uint32 h);
		const Rect& GetRect() const;

		void SetWidget(Widget* widget);
		Widget* GetWidget() const;

		void Draw(sf::RenderWindow& window) const;

		bool empty() const;

		bool IsExpand() const;
		void SetExpand(bool flag);

		uint32 GetRowSpan() const;
		uint32 GetColSpan() const;

		void SetRowSpan(uint32 count);
		void SetColSpan(uint32 count);

		void ResizeWidget(uint32 panning);
		void UpdateWidgetPos(uint32 panning);
	private:
		Rect m_rect;
		sf::Shape m_shape;
		Widget* m_widget;
		bool m_expanded;

		uint32 m_rowspan;
		uint32 m_colspan;
	};

	class GridLayout : public Widget
	{
	public:
		GridLayout();
		~GridLayout();

		bool AddWidgetToGrid(Widget* widget, uint32 line, uint32 column);
		Widget* RemoveWidgetAt(uint32 line, uint32 column);

		void AddLineBefore(uint32 line);
		void AddLineAfter(uint32 line);

		void AddColumnBefore(uint32 column);
		void AddColumnAfter(uint32 column);

		bool RemoveLineIfEmpty(uint32 line);	//returns true if deleted the line
		bool RemoveColumnIfEmpty(uint32 column);//returns true if deleted the col

		void RemoveEmptyColumnAndLines();

		bool IsLineEmpty(uint32 line) const;
		bool IsColumnEmpty(uint32 column) const;

		bool HandleDragDrop(Drag* drag);
		
		void SetPanning(uint32 panning);
		uint32 GetPanning() const;
	private:
		bool IsExtremity(uint32 row, uint32 col) const;
		void ComputeCells();					//internally used to resize the cells
		bool AddWidget(Widget* child);			//only internally used
		bool RemoveWidget(Widget* widget);		//internally used as well

		void RemoveWidgetFromGrid(Widget* widget);
		void Draw() const;

		void SetPos(int x, int y, bool forceMove /* = false */, bool save /* = true */);
		void SetPosForGrid();
		bool HandleDragStop(Drag* drag);

		bool FindWidgetInGrid(Widget* widget, uint32& line, uint32& column);
		LayoutItem::CollisionType FindGridLocationAt(int32 xpos, int32 ypos, uint32& line, uint32& column);

		void ReloadSettings();
		
		void SaveGridProperties();
		void LoadGridProperties();
		//lines		//columns
		std::vector<std::vector<LayoutItem> > m_items;
		uint32 m_panning;						//panning for widgets
	};


}