#include "../include/gui/GridLayout.hpp"

namespace gui
{
	void GridLayout::AddWidgetToGrid( Widget* widget, uint32 line, uint32 column )
	{
		AddWidget(widget);

	}

	Widget* GridLayout::RemoveWidgetAt( uint32 line, uint32 column )
	{
		if(line > m_items.size()) return NULL;
		if(column > m_items[line].size()) return NULL;

		LayoutItem layout_item = m_items[line][column];
		Widget* widget = layout_item.GetWidget();
		RemoveWidget(widget);

		m_items[line].erase(m_items[line].begin()+column);

		return widget;
	}

	GridLayout::GridLayout()
	{
		m_type = GRID_LAYOUT;
		m_panning = 2;		//by default 2 pixels pannning

		//resize to a 2 by 2 grid
		m_items.resize(2);
		for(uint32 i=0; i<m_items.size(); i++) {
			m_items[i].resize(2);
		}
	}

	void GridLayout::HandleDragDrop( Drag* drag )
	{
		if(!drag || drag->GetType() != Drag::Widget) 
			return;

		//calculate in which cell to drop the widget
	}

	void GridLayout::ComputeCells()
	{
		const uint32 min_empty_size = 15; //give an empty cell at least 15px w/h. 2*5px for margins, 5px for proper

		uint32 width = m_rect.w;
		uint32 height = m_rect.h;

		uint32 curWidth = width;
		uint32 curHeight = height;

		//uint32 fixedCount = 0;
		uint32 minCountWidth  = 0;
		uint32 maxCountWidth  = 0;
		uint32 minCountHeight = 0;
		uint32 maxCountHeight = 0;

		std::vector<uint32> remainingWidths;		//remaining width per line. 0 if none remained or was negative
		remainingWidths.resize(m_items.size());

		//initialize all cells with equal widths & heights
		uint32 default_width = m_items.size() ? m_items[0].size() ? m_rect.w/ m_items.size() : m_rect.w
											  : 1;
		uint32 default_height = m_items.size() ? m_rect.h / m_items.size() : 1;

		Rect default_size(0,0,default_width,default_height);
		
		for(uint32 i=0; i<m_items.size(); i++) {
			for(uint32 j=0; j<m_items[i].size(); j++) {
				m_items[i][j].SetSize(default_size);
			}
		}

		error_log("Default size: %d, %d: ", default_size.w, default_size.h);

		//find out the remaining width if any
		for(uint32 i=0; i<m_items.size(); i++) {
			std::vector<LayoutItem>& v = m_items[i];
			for(uint32 j=0; j<v.size(); j++) {
				LayoutItem& item = v[j];
				Widget* widget = item.GetWidget();
				if(!widget) {
					curWidth += min_empty_size;
					continue;
				}
				curWidth += widget->GetSizeHint().x;
			}
			//if the current line's width exceeds that of that parent widget, scaled resize the columns
			if(curWidth > this->m_rect.w) {
				float scale = m_rect.w / (float)curWidth;
				for(uint32 j=0; j<v.size(); j++) {
					//v[j].
				}
			}
		}

		//count the types of size policies
		for(uint32 i=0; i<m_items.size(); i++) 
		{
			std::vector<LayoutItem>& v = m_items[i];
			for(uint32 j=0; j<v.size(); j++) 
			{
				LayoutItem& item = v[j];
				
				//if you empty make sure you get at least min_empty_size width/height
				if(item.empty()) {
					curHeight += min_empty_size;
					curWidth += min_empty_size;
					continue;
				}

				uint32 vpolicy = item.GetWidget()->GetVerticalPolicy();
				uint32 hpolicy = item.GetWidget()->GetHorizontalPolicy();

				//get the columns actual vertical size policy
				for(uint32 k=0; k<m_items.size(); k++) {
					Widget* widget = m_items[k][j].GetWidget();
					if(!widget) continue;

					if(vpolicy == MaximumExpand) break;	//you're already attempting to expand at max. capacity
					else if(widget->GetVerticalPolicy() == MaximumExpand) {
						vpolicy = MaximumExpand;
					}
				}
				//get the columns actual vertical size policy
				for(uint32 k=0; k<m_items.size(); k++) {
					Widget* widget = m_items[k][j].GetWidget();
					if(!widget) continue;

					if(hpolicy == MaximumExpand) break;	//you're already attempting to expand at max. capacity
					else if(widget->GetHorizontalPolicy()  == MaximumExpand) {
						hpolicy = MaximumExpand;
					}
				}

				// Min Expand case
				if(hpolicy == Widget::MinimumExpand) {
					minCountWidth++;
				}
				if(vpolicy == Widget::MinimumExpand) {
					minCountHeight++;
				}
				// Max Expand case
				if(hpolicy == Widget::MaximumExpand) {
					maxCountWidth++;
				}
				if(vpolicy == Widget::MaximumExpand) {
					maxCountHeight++;
				}
				// Fixed case
			}
		}
	}

	void GridLayout::SetPanning( uint32 panning )
	{
		m_panning = panning;
	}

	uint32 GridLayout::GetPanning() const
	{
		return m_panning;
	}

	void LayoutItem::SetWidget( Widget* widget )
	{
		m_widget = widget;
	}

	const Rect& LayoutItem::GetSize() const
	{
		return m_rect;
	}

	Widget* LayoutItem::GetWidget() const
	{
		return m_widget;
	}

	LayoutItem::CollisionType LayoutItem::IsCollision( int x, int y ) const
	{
		Rect temp(x,y,1,1);
		if(!gui::IsCollision(temp,m_rect)) {
			return LayoutItem::NoCollision;
		}

		Rect proper = m_rect - 5;	//5px panning

		if(gui::IsCollision(temp,proper)) {
			return LayoutItem::ProperCollision;
		}


		/* There's is a collision but it's outside of the proper rect! */

		//upper collision
		if(y < proper.y) 
			return LayoutItem::UpperCollision;
		
		//lower collision
		if(y > proper.y + proper.h)
			return LayoutItem::LowerCollision;
		
		//left collision
		if(x < proper.x) 
			return LayoutItem::LeftCollision;

		//right collision
		if(x < proper.x + proper.w)
			return LayoutItem::RightCollision;

		return LayoutItem::NoCollision;	//this should never happen
	}

	bool LayoutItem::empty() const
	{
		return (m_widget == NULL);
	}
}
