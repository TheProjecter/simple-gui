#include "../include/gui/GridLayout.hpp"
#include "../include/gui/GuiManager.hpp"
#include <sstream>

namespace gui
{
	GridLayout::GridLayout()
	{
		m_type = GRID_LAYOUT;
		m_panning = 2;		//by default 5 pixels panning

		//resize to a 1 by 1 grid
		m_items.resize(1);
		for(uint32 i=0; i<m_items.size(); i++) {
			m_items[i].resize(1);
		}

		ComputeCells();
	}

	GridLayout::~GridLayout()
	{
		//nothing to do really...
	}
	bool GridLayout::AddWidgetToGrid( Widget* widget, uint32 line, 
			uint32 column, uint32 rowspan/*=1*/, uint32 colspan/*=1*/ )
	{
		//assert(widget && m_items.size() > line && m_items[line].size() > column);

		if(line + rowspan > m_items.size()) {
			m_items.resize(line + rowspan);
			uint32 maxcols = 0;
			//get the biggest column count
			for(uint32 i=0; i<m_items.size(); i++) {
				maxcols = std::max(maxcols,m_items[i].size());
			}
			//resize them all to be the same no. of columns if needed
			for(uint32 i=0; i<m_items.size(); i++) {
				if(m_items[i].size() != maxcols) {
					m_items[i].resize(maxcols);
				}
			}
		}
		if( column + colspan > m_items[0].size()) {
			for(uint32 i=0; i < m_items.size(); i++) {
				m_items[i].resize(column + colspan);
			}
		}

		LayoutItem& layout_item = m_items[line][column];
		if(!layout_item.empty() || layout_item.IsExpand()) {
			return false;
		}

		//Widget::AddWidgetForced(widget);
		
		if(!Widget::AddWidget(widget)) 
			return false;

		//update every line we *own* to notify they're no longer occupied
		for(uint32 k=line+1; k<line+layout_item.GetRowSpan(); k++) {
			m_items[k][column].SetExpand(false);
		}

		//update every column we *own* to notify they're no longer occupied
		for(uint32 k=column+1; k<column+layout_item.GetColSpan(); k++) {
			m_items[line][k].SetExpand(false);
		}

		layout_item = LayoutItem(widget,rowspan,colspan);


		//update every line we *own* to notify they're no longer occupied
		for(uint32 k=line+1; k<line+layout_item.GetRowSpan(); k++) {
			m_items[k][column].SetExpand(true);
		}

		//update every column we *own* to notify they're no longer occupied
		for(uint32 k=column+1; k<column+layout_item.GetColSpan(); k++) {
			m_items[line][k].SetExpand(true);
		}

		

		//adding widgets can also lead to useless redundant rows/columns!
		RemoveEmptyColumnAndLines();	
		SaveGridProperties();
		ComputeCells(); //recalculate cells
		return true;
	}

	Widget* GridLayout::RemoveWidgetAt( uint32 line, uint32 column )
	{
		if(line > m_items.size()) return NULL;
		if(column > m_items[line].size()) return NULL;

		LayoutItem layout_item = m_items[line][column];
		Widget* widget = layout_item.GetWidget();
		RemoveWidget(widget);

		//resize the grid if there are empty useless spaces
		RemoveEmptyColumnAndLines();

		ComputeCells();

		return widget;
	}

	bool GridLayout::HandleDragDrop( Drag* drag )
	{
		if(!drag || drag->GetType() != Drag::Widget) 
			return false;

		bool collision = false;
		uint32 cellRow = 0;
		uint32 cellCol = 0;

		int32 xpos = (int32)drag->GetCurrentMousePos().x;
		int32 ypos = (int32)drag->GetCurrentMousePos().y;

		//calculate in which cell to drop the widget
		for(uint32 i=0; i<m_items.size(); i++) {
			for(uint32 j=0; j<m_items[i].size(); j++) {
				LayoutItem& layout_item = m_items[i][j];
				Rect old = layout_item.GetRect();
				Rect new_rect = old;
				bool need_resize = false;

				if(layout_item.GetRowSpan() > 1 || layout_item.GetColSpan() > 1) {
					need_resize = true;
					//take in account for multiple columns expanded cells
					if(layout_item.GetColSpan() > 1) {
						uint32 extra_width = 0;
						for(uint32 k=j+1; k<j+layout_item.GetColSpan(); k++) {
							LayoutItem& cell = m_items[i][k];
							extra_width += cell.GetRect().w;
						}
						new_rect.w -= extra_width;
					}
				}
				//take in account for multiple columns expanded cells
				if(layout_item.GetRowSpan() > 1) {
					uint32 extra_height = 0;
					for(uint32 k=i+1; k<i+layout_item.GetRowSpan(); k++) {
						LayoutItem& cell = m_items[k][0];
						extra_height += cell.GetRect().h;
					}
					new_rect.h -= extra_height;
				}
				if(need_resize) {
					layout_item.SetSize(new_rect.w, new_rect.h, IsExtremity(i,j));
				}

				switch(layout_item.IsCollision(xpos,ypos,m_panning)) {
					case LayoutItem::NoCollision: break;
					case LayoutItem::ProperCollision:
						collision = true;
						cellRow = i;
						cellCol = j;
						break;
					case LayoutItem::UpperCollision:
						AddLineBefore(i);	//the line i will become i+1, and the line we will add the widget is i!
						collision = true;
						cellRow = i;
						cellCol = j;
						break;
					case LayoutItem::LowerCollision:
						AddLineAfter(i);	//the line i will remain i, and the line we will add the widget is i+1!
						collision = true;
						cellRow = i+1;
						cellCol = j;
						break;
					case LayoutItem::LeftCollision:
						AddColumnBefore(j);	//the col j will become j+1, and the col we will add the widget is j!
						collision = true;
						cellRow = i;
						cellCol = j;
						break;
					case LayoutItem::RightCollision:
						AddColumnAfter(j);//the col j will remain j, and the col we will add the widget is j+1!
						collision = true;
						cellRow = i;
						cellCol = j+1;
						break;
				}
				if(need_resize) {
					layout_item.SetSize(old.w,old.h,IsExtremity(i,j));
				}

				if(collision) break;	//else it may crash because we might have changed indexes when adding a widget
			}
			
			if(collision) break;	//else it may crash because we might have changed indexes when adding a widget
		}
		if(collision) {
			error_log("Collision at grid location: %u:%u", cellRow, cellCol);
			if(!AddWidgetToGrid(drag->GetTarget(),cellRow,cellCol)) {
				return false;
			}

			return true;
		}

		return false;	//shouldn't ever happen...
	}

	void GridLayout::ComputeCells()
	{
		if(!m_items.size()) {
			error_log("This shouldn't happen... there are no items in the grid!");
			return;
		}

		std::vector<uint32> maxcols;	//columns which have MaximumExpaned Horizontal policy
		std::vector<uint32> mincols;	//columns which have MinimumExpaned Horizontal policy

		std::vector<uint32> maxrows; //rows which have MaximumExpaned Vertical policy
		std::vector<uint32> minrows; //rows which have MinimumExpaned Vertical policy

		//				 width/height,  policy
		typedef std::pair<uint32,	 SizePolicy> SizePolicyPair;
		std::vector<SizePolicyPair> columnsInfo;
		std::vector<SizePolicyPair> rowsInfo;

		columnsInfo.resize(m_items[0].size());
		rowsInfo.resize(m_items.size());

		//iterate through columns. I assume on every lines there's the same number of columns.. otherwise CRASH!
		for(uint32 i=0; i<m_items[0].size(); i++) {
			//iterate through every line of that column
			for(uint32 j=0; j<m_items.size(); j++) {
				LayoutItem& layout_item = m_items[j][i];
				
				//update the column info

				//there's no widget and the minimum size hasn't been set yet
				if(!layout_item.GetWidget()) {
					if(columnsInfo[i].first < 15)
						columnsInfo[i].first = 15;	//default 15px min width

					continue;	//nothing to do if there's no widget
				}
				//the size hint of the current column item is bigger than the current width of the column
				if(layout_item.GetWidget()->GetSizeHint().x > (int)columnsInfo[i].first) {
					columnsInfo[i].first = layout_item.GetWidget()->GetSizeHint().x;
				}

				//update size policy of the current column.. if needed
				switch (layout_item.GetWidget()->GetHorizontalPolicy())
				{
				case MinimumExpand:
					//don't worry about checking if it's already in maxcols.. if it is then mincols is redundant
					// 
					//add the columns to the min columns if it wasn't there already
					if(std::find(mincols.begin(), mincols.end(), i) == mincols.end()) {
						mincols.push_back(i);	
					}
					//if column less than minimum or maximum, update
					if((int)columnsInfo[i].second < layout_item.GetWidget()->GetHorizontalPolicy()) {
						columnsInfo[i].second = layout_item.GetWidget()->GetHorizontalPolicy();
					}
					break;

				case MaximumExpand:
					//if column less than minimum or maximum, update. (should only happen once per col.. but check anyway?)
					if(columnsInfo[i].second < layout_item.GetWidget()->GetHorizontalPolicy()) {
						columnsInfo[i].second = layout_item.GetWidget()->GetHorizontalPolicy();

						//don't worry about deleting the *maybe* existing entry from mincols.. 
						//if there's a maxcols then mincols will be useless
						if(std::find(maxcols.begin(), maxcols.end(), i) == maxcols.end()) {
							maxcols.push_back(i);	//this check may be useless.. since it should only happen once
						}
					}
					
					break;

				default: break;
				}
			}
		}
		/* Finished checking widths/policies. Adjust sizes */

		uint32 columnWidth = 0;

		for(uint32 i=0; i<columnsInfo.size(); i++) {
			//15px is the minimum size
			if(columnsInfo[i].first < 15) {
				columnsInfo[i].first = 15;
			}
			columnWidth += columnsInfo[i].first;
		}
		//adjust the width for the panning distance
		uint32 panWidth = (m_panning * 2 * columnsInfo.size()); //2 pannings per item..  

		//when checking for the diff take in account the panning
		int32 diff = m_rect.w - (columnWidth + panWidth);

		//if the column's width is bigger than the rect.. must resize the columns
		if(diff < 0) {
			diff += panWidth;	//the columns should be as wide as the widget.. 
			float coeff = m_rect.w / (float) (columnWidth);	//resize all columns by coeff to fit my size

			for(uint32 i=0; i<columnsInfo.size(); i++) {
				columnsInfo[i].first = uint32(columnsInfo[i].first * coeff);
			}
		} else {	//there's extra space

			diff += panWidth;	//the columns should be as wide as the widget.. 
			
			//some columns require all the space available
			if(maxcols.size()) {
				uint32 space = diff / maxcols.size();
				for(uint32 i=0; i<maxcols.size(); i++) {
					columnsInfo[maxcols[i]].first += space;
				}
			} else if(mincols.size()) { //there's no maxcols, mincols can have the extra space..
				uint32 space = diff / mincols.size();
				for(uint32 i=0; i<mincols.size(); i++) {
					columnsInfo[mincols[i]].first += space;
				}
			} else { //no max.. no min.. just split the extra space equally between columns
				uint32 space = diff / columnsInfo.size();
				for(uint32 i=0; i<columnsInfo.size(); i++) {
					columnsInfo[i].first += space;
				}
			}
		}

		/* Finished width.. now on to height */

		//iterate through columns. I assume on every lines there's the same number of columns.. otherwise CRASH!
		for(uint32 i=0; i<m_items.size(); i++) {
			//iterate through every line of that column
			for(uint32 j=0; j<m_items[i].size(); j++) {
				LayoutItem& layout_item = m_items[i][j];

				//update the column info

				//there's no widget and the minimum size hasn't been set yet
				if(!layout_item.GetWidget()) {
					if(rowsInfo[i].first < 15)
						rowsInfo[i].first = 15;	//default 15px min height

					continue;	//nothing to do if there's no widget
				}
				//the size hint of the current column item is bigger than the current width of the column
				if(layout_item.GetWidget()->GetSizeHint().y > (int)rowsInfo[i].first) {
					rowsInfo[i].first = layout_item.GetWidget()->GetSizeHint().y;
				}

				//update size policy of the current column.. if needed
				switch (layout_item.GetWidget()->GetVerticalPolicy())
				{
				case MinimumExpand:
					//don't worry about checking if it's already in maxcols.. if it is then mincols is redundant
					// 
					//add the columns to the min rows if it wasn't there already
					if(std::find(minrows.begin(), minrows.end(), i) == minrows.end()) {
						mincols.push_back(i);	
					}
					//if column less than minimum or maximum, update
					if(rowsInfo[i].second < layout_item.GetWidget()->GetVerticalPolicy()) {
						rowsInfo[i].second = layout_item.GetWidget()->GetVerticalPolicy();
					}
					break;

				case MaximumExpand:
					//if column less than minimum or maximum, update. (should only happen once per col.. but check anyway?)
					if(rowsInfo[i].second < layout_item.GetWidget()->GetVerticalPolicy()) {
						rowsInfo[i].second = layout_item.GetWidget()->GetVerticalPolicy();

						//don't worry about deleting the *maybe* existing entry from minrows.. 
						//if there's a maxcols then minrows will be useless
						if(std::find(maxrows.begin(), maxrows.end(), i) == maxrows.end()) {
							maxrows.push_back(i);	//this check may be useless.. since it should only happen once
						}
					}

					break;

				default: break;
				}
			}
		}
		/* Finished checking height/policies. Adjust sizes */

		uint32 rowHeight = 0;

		for(uint32 i=0; i<rowsInfo.size(); i++) {
			//15px is the minimum size
			if(rowsInfo[i].first < 15) {
				rowsInfo[i].first = 15;
			}
			rowHeight+= rowsInfo[i].first;
		}
		//adjust the width for the panning distance
		uint32 panHeight = (m_panning * 2 * rowsInfo.size());

		diff = m_rect.h - (rowHeight + panHeight);

		//if the column's width is bigger than the rect.. must resize the columns
		if(diff < 0) {
			diff += panHeight;	//the columns should be as wide as the widget.. 
			float coeff = m_rect.h / (float) (rowHeight);	//resize all columns by coeff to fit my size

			for(uint32 i=0; i<rowsInfo.size(); i++) {
				rowsInfo[i].first = uint32(rowsInfo[i].first * coeff);
			}
		} else {	//there's extra space

			diff += panHeight;	//the columns should be as wide as the widget.. 
			//some columns require all the space available
			if(maxrows.size()) {
				uint32 space = diff / maxrows.size();
				for(uint32 i=0; i<maxrows.size(); i++) {
					rowsInfo[maxrows[i]].first += space;
				}
			} else if(minrows.size()) { //there's no maxcols, mincols can have the extra space..
				uint32 space = diff / minrows.size();
				for(uint32 i=0; i<minrows.size(); i++) {
					rowsInfo[minrows[i]].first += space;
				}
			} else { //no max.. no min.. just split the extra space equally between columns
				uint32 space = diff / rowsInfo.size();
				for(uint32 i=0; i<rowsInfo.size(); i++) {
					rowsInfo[i].first += space;
				}
			}
		}
		uint32 xpos = m_rect.x;
		uint32 ypos = m_rect.y;

		/* Finished width and height.. Set the size of the individual grid items */
		for(uint32 i=0; i<m_items.size(); i++) {
			xpos = m_rect.x;	//restart at 0 every line

			for(uint32 j=0; j<m_items[i].size(); j++) {
				LayoutItem& layout_item = m_items[i][j];
				
				uint32 rowspan = layout_item.GetRowSpan();
				uint32 colspan = layout_item.GetColSpan();

				uint32 width  = 0;	//start from 0, since rowspan will be 1 by default.. so the width will be added
				uint32 widthSpan = 0;

				width += columnsInfo[j].first;
				//make sure you have the correct width for row-spanned items
				for(uint32 k=j; k<j+colspan; k++) {
					widthSpan += columnsInfo[k].first;
				}
				uint32 height = 0;
				uint32 heightSpan = 0;

				height += rowsInfo[i].first;
				//make sure you have the correct height for row-spanned items
				for(uint32 k=i; k<i+rowspan; k++) {
					heightSpan += rowsInfo[k].first;
				}

				layout_item.SetSize(widthSpan,heightSpan,IsExtremity(i,j));
				layout_item.SetPos(xpos,ypos);

				//if it span over multiple columns..move by width.
				xpos += width;	//go to the next item


				/* Finished setting the size of the grid items.. now resize the widgets! */
				layout_item.ResizeWidget(m_panning);
				layout_item.UpdateWidgetPos(m_panning);
			}
			ypos += rowsInfo[i].first;	//go to the next item


		}

	}

	void GridLayout::SetPosForGrid()
	{
		uint32 xpos = m_rect.x;
		uint32 ypos = m_rect.y;

		for(uint32 i=0; i<m_items.size(); i++) {
			xpos = m_rect.x;
			for(uint32 j=0; j<m_items[i].size(); j++) {
				LayoutItem& layout_item = m_items[i][j];
				
				layout_item.SetPos(xpos,ypos);
				xpos += layout_item.GetRect().w;

				//take in account for multiple columns expanded cells
				if(layout_item.GetColSpan() > 1) {
					uint32 extra_width = 0;
					for(uint32 k=j+1; k<j+layout_item.GetColSpan(); k++) {
						LayoutItem& cell = m_items[i][k];
						extra_width += cell.GetRect().w;
					}
					xpos -= extra_width;
				}



				layout_item.UpdateWidgetPos(m_panning);
			}
			LayoutItem& layout_item = m_items[i][0];

			//take in account for multiple columns expanded cells
			if(layout_item.GetRowSpan() > 1) {
				uint32 extra_height = 0;
				for(uint32 k=i+1; k<i+layout_item.GetRowSpan(); k++) {
					LayoutItem& cell = m_items[k][0];
					extra_height += cell.GetRect().h;
				}
				ypos -= extra_height;
			}

			ypos += m_items[i][0].GetRect().h;
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

	void GridLayout::AddLineBefore( uint32 line )
	{
		if(!m_items.size()) {
			error_log("Grid is empty!");
			return;
		}
		if(line > m_items.size()) {
			error_log("Line is outside the bound of the grid! line=%u, size=%u",line, m_items.size());
			return;
		}

		std::vector<LayoutItem> temp;
		temp.resize(m_items[0].size());	//resize to grid's size

		m_items.insert(m_items.begin()+line,temp);
	}
	void GridLayout::AddLineAfter( uint32 line )
	{
		if(!m_items.size()) {
			error_log("Grid is empty!");
			return;
		}
		if(line > m_items.size()) {
			error_log("Line is outside the bound of the grid! line=%u, size=%u",line, m_items.size());
			return;
		}

		line++; //add the line after this one..

		std::vector<LayoutItem> temp;
		temp.resize(m_items[0].size());	//resize to grid's size

		m_items.insert(m_items.begin()+line,temp);
	}

	void GridLayout::AddColumnBefore( uint32 column )
	{
		//assume the grid has the same number of columns
		if(!m_items.size()) {
			error_log("Grid is empty!");
			return;
		}

		//add a new column on each line before the specified column
		for(uint32 i=0; i<m_items.size(); i++ ) {
			m_items[i].insert(m_items[i].begin()+column,LayoutItem());
		}
	}
	void GridLayout::AddColumnAfter( uint32 column )
	{
		//assume the grid has the same number of columns
		if(!m_items.size()) {
			error_log("Grid is empty!");
			return;
		}

		//add a new column on each line after the specified column
		for(uint32 i=0; i<m_items.size(); i++ ) {
			m_items[i].insert(m_items[i].begin()+column+1,LayoutItem());
		}
	}

	//This should only get called by GuiManager when loading the .ui
	//Since the properties will be loaded before the actual widgets,
	//that means that the grid properties can be used to tell this
	//function how/where to(in the grid) place the widgets
	// NEED TO FIX THIS!!
	bool GridLayout::AddWidget( Widget* child )
	{
		if(!child) 
			return false;

// 		SaveGridProperties();
		std::string name = "grid-" + child->GetName();
		
		//since this function should only get called by the manager
		//when loading the ui.. the grid should have it's properties
		//set with the location of its widgets in the grid, 
		//and the col/row span... else.. something went wrong?
		if(!m_settings.HasStringValue(name)) {
			error_log("Widget(\"%s\") doesn't have any grid properties set!",child->GetName().c_str());
			return false;
		}
		std::string temp = m_settings.GetStringValue(name);
		std::stringstream s;
		s.str(temp);

		uint32 cellRow(0), cellCol(0), rowSpan(1), colSpan(1);
		s >> cellRow >> cellCol >> rowSpan >> colSpan;

		if(s.bad()) {
			error_log("Error when reading grid properties for widget: %s", child->GetName().c_str());
			return false;
		}

		AddWidgetToGrid(child,cellRow,cellCol);

		//should make some error checking.. to make sure the row/cols are 
		//empty before setting them.. 
		m_items[cellRow][cellCol].SetRowSpan(rowSpan);
		m_items[cellRow][cellCol].SetColSpan(colSpan);

		
		for(uint32 i=cellRow+1; i<cellRow+rowSpan; i++) {
			for(uint32 j=cellCol+1; j<cellCol+colSpan; j++) {
				m_items[i][j].SetExpand(true);
			}
			m_items[i][cellCol].SetExpand(true);
		}

		for(uint32 j=cellCol+1; j<cellCol+colSpan; j++) {
			m_items[cellRow][j].SetExpand(true);
		}

		return true;
	}

	void GridLayout::SaveGridProperties()
	{
		if(!m_items.size()) {
			error_log("Grid is empty!");
			return;
		}

		std::stringstream s;
		s << m_items.size() << " " << m_items[0].size();
		m_settings.SetStringValue("grid-properties",s.str());
		s.str(std::string());
		
		for(uint32 i=0; i<m_items.size(); i++) {
			for(uint32 j=0; j<m_items[i].size(); j++) {
				LayoutItem& layout_item = m_items[i][j];
				if(layout_item.empty()) continue;

				std::string name = "grid-" + layout_item.GetWidget()->GetName();
				s << i << " " << j << " " << layout_item.GetRowSpan() << " " << layout_item.GetColSpan();
				std::string value = s.str();
				s.str(std::string());
				m_settings.SetStringValue(name,value);
			}
		}
	}

	void GridLayout::LoadGridProperties()
	{
		if(!m_settings.HasStringValue("grid-properties")) {
			error_log("Grid doesn't not have properties! Cannot continue!");
			return;
		}
		std::stringstream s;
		s.str(m_settings.GetStringValue("grid-properties"));

		uint32 rowsize(0), colsize(0);
		s >> rowsize >> colsize;

		m_items.resize(rowsize);
		for(uint32 i=0; i<rowsize; i++) {
			m_items[i].resize(colsize);
		}

	}

	bool GridLayout::RemoveWidget( Widget* widget )
	{
		//remove the widget from management
		if(!Widget::RemoveWidget(widget)) {
			error_log("Can't remove widget from grid!");
			return false;
		}
		//now remove widget from grid
		RemoveWidgetFromGrid(widget);

		//fix redundant rows/cols
		RemoveEmptyColumnAndLines();
		
		return true;	//what if something went wrong?..

	}

	void GridLayout::RemoveWidgetFromGrid( Widget* widget )
	{
		if(!widget) return;

		uint32 line = 0;
		uint32 column = 0;

		if(!FindWidgetInGrid(widget,line,column)) {
			return;
		}

		LayoutItem& layout_item = m_items[line][column];

		//update every line we *own* to notify they're no longer occupied
		for(uint32 k=line+1; k<line+layout_item.GetRowSpan(); k++) {
			m_items[k][column].SetExpand(false);
		}

		//update every column we *own* to notify they're no longer occupied
		for(uint32 k=column+1; k<column+layout_item.GetColSpan(); k++) {
			m_items[line][k].SetExpand(false);
		}

		layout_item.SetWidget(NULL);
		
		layout_item.SetColSpan(1);
		layout_item.SetRowSpan(1);
		//RemoveEmptyColumnAndLines();
		
		//ComputeCells();	//don't compute? will be handled somewhere else.. you just worry about the grid!
	}

	void GridLayout::ReloadSettings()
	{
		Widget::ReloadSettings();

		LoadGridProperties();
	}

	void GridLayout::Draw() const
	{
		if(!m_visible) return;

		Widget::Draw();
		
		for(uint32 i=0; i<m_items.size(); i++) {
			for(uint32 j=0; j<m_items[i].size(); j++) {
				m_items[i][j].Draw(s_gui->GetWindow());
			}
		}

	}

	void GridLayout::SetPos( int x, int y, bool forceMove /* = false */, bool save /* = true */ )
	{
		Widget::SetPos(x,y,forceMove,save);

		SetPosForGrid();
	}

	bool GridLayout::IsLineEmpty(uint32 line) const
	{
		if(line > m_items.size()) {
			error_log("Line is outside grid bounds! %u", line);
			return false;
		}
		for(uint32 j=0; j<m_items[line].size(); j++) {
			//cell doesn't contain a widget, and is not an extend of anothers
			if(!m_items[line][j].empty() || m_items[line][j].IsExpand()) {
				return false;
			}
		}

		return true;	
	}

	bool GridLayout::IsColumnEmpty( uint32 column ) const
	{
		if(!m_items.size()) {
			error_log("Grid is empty!");
			return false;
		}

		//assume grid has the same number of columns.. otherwise it may crash!
		if(column > m_items[0].size()) {
			error_log("Column is outside grid bounds! %u", column);
			return false;
		}

		for(uint32 i=0; i<m_items.size(); i++) {
			if(!m_items[i][column].empty() || m_items[i][column].IsExpand()) {
				return false;
			}
		}

		return true;	
	}

	bool GridLayout::RemoveLineIfEmpty( uint32 line )
	{
		if(!IsLineEmpty(line)) 
			return false;

		//don't delete anything if the grid if the size specified
		//in the settings.. would cause crashes!
		if(m_settings.HasStringValue("grid-properties") && IsLoading()) {
			std::string temp = m_settings.GetStringValue("grid-properties");
			std::stringstream s(temp);
			uint32 rows(0), cols(0);
			s >> rows >> cols;
			if(m_items.size() <= rows) {
				return false;
			}
		}

		//don't delete anything if the grid is smaller than 1x1
		if(m_items.size() <= 1) 
			return false;

		//remove the line!
		m_items.erase(m_items.begin()+line);


		ComputeCells();	//needs a recompute!
		return true;
	}

	bool GridLayout::RemoveColumnIfEmpty( uint32 column )
	{
		if(!IsColumnEmpty(column)) 
			return false;

		//don't delete anything if the grid if the size specified
		//in the settings.. would cause crashes!
		if(m_settings.HasStringValue("grid-properties") && IsLoading()) {
			std::string temp = m_settings.GetStringValue("grid-properties");
			std::stringstream s(temp);
			uint32 rows(0), cols(0);
			s >> rows >> cols;
			if(m_items[0].size() <= cols) {
				return false;
			}
		}

		//don't delete anything if the grid is smaller than 1x1
		if(m_items[0].size() <= 1) 
			return false;


		//remove the column
		for(uint32 i=0; i<m_items.size(); i++) {
			m_items[i].erase(m_items[i].begin()+column);
		}

		ComputeCells();	//needs a recompute!
		return true;
	}

	void GridLayout::RemoveEmptyColumnAndLines()
	{
		for(uint32 i=0; i<m_items.size(); i++) {
			
			if(RemoveLineIfEmpty(i)) {
				i = 0;
				continue;
			}
			for(uint32 j=0; j<m_items[i].size(); j++) {
				if(RemoveColumnIfEmpty(j)) {
					j = 0;	//go back to the start to avoid crash
				}
			}

		}
	}

	bool GridLayout::HandleDragStop( Drag* drag )
	{
		if(!drag || drag->GetType() != Drag::Widget) 
			return false;


		//if you're not the target where the drop will happen.. then the 
		//widget specific function can take care of the drop
		if(drag->GetCurrentFocus() != this) {
			if(Widget::HandleDragStop(drag)) {
				//now that the widget is no longer managed by the grid.. 
				// return to your default size
				uint32 width = drag->GetTarget()->GetSizeHint().x;
				uint32 height = drag->GetTarget()->GetSizeHint().y;

				//otherwise .. it will reset the size of the widget..
				//after it was already calculated...
				if(drag->GetTarget()->GetParent() && 
				   drag->GetTarget()->GetParent()->GetType() != GRID_LAYOUT)
				{
					drag->GetTarget()->Resize(width, height);
				}

				//now you can remove the extra *if any* rows and/or columns
				RemoveEmptyColumnAndLines();

				ComputeCells(); //recompute? I'm pretty sure it's needed

				return true;
			} else {
				drag->ResetPosition();			
				return false;
			}
			
		}

		//if you're still the target of the drop.. you need to handle it 
		//differently than the basic widget, because you need to change
		//it's location in the grid!

		uint32 line = 0;
		uint32 column = 0;
		if(!FindWidgetInGrid(drag->GetTarget(), line, column)) {
			error_log("GridLayout initiated drag for(%s), but doesn't contain it in the grid!", drag->GetTarget()->GetName().c_str());
			//add the widget in it's new location?
			if(!FindGridLocationAt((uint32)drag->GetCurrentMousePos().x, 
				(uint32)drag->GetCurrentMousePos().y, line, column)) 
			{
				error_log("Couldn't add widget to grid! Invalid coords(not inside the grid!)x: %u|y: %u",drag->GetCurrentMousePos().x, drag->GetCurrentMousePos().y);
				drag->ResetPosition();
				return false;
			} 
			LayoutItem& layout_item = m_items[line][column];
			if(!layout_item.empty() || layout_item.IsExpand()) {
				error_log("Tried to add widget(%s) to grid(%u,%u).. but it was occupied!",drag->GetTarget()->GetName().c_str(),line,column);			
				drag->ResetPosition();
				return false;
			}
			layout_item.SetWidget(drag->GetTarget());
			return true;
		}
		uint32 old_widget_line = line;
		uint32 old_widget_col  = column;

		//find it's new location in the grid
		if(!FindGridLocationAt((uint32)drag->GetCurrentMousePos().x, 
			(uint32)drag->GetCurrentMousePos().y, line, column)) 
		{
			error_log("Couldn't add widget to grid! Invalid coords(not inside the grid!)x: %u|y: %u",drag->GetCurrentMousePos().x, drag->GetCurrentMousePos().y);
			drag->ResetPosition();
			return false;
		} 

		LayoutItem& layout_item = m_items[line][column];

		if(!layout_item.empty() || layout_item.IsExpand()) {
			error_log("Tried to add widget(%s) to grid(%u,%u).. but it was occupied!",drag->GetTarget()->GetName().c_str(),line,column);			
			drag->ResetPosition();
			return false;
		}

		//remove it after you have some valid coords in the grid
		RemoveWidgetFromGrid(drag->GetTarget());	
		
		layout_item.SetWidget(drag->GetTarget());		

		//now you can remove the extra *if any* rows and/or columns
		RemoveEmptyColumnAndLines();
		
		ComputeCells(); //recompute? I'm pretty sure it's needed

		return true;
	}

	bool GridLayout::FindWidgetInGrid( Widget* widget, uint32& line, uint32& column )
	{
		//to avoid saying we found it in empty grids..
		if(!widget) return false;	

		for(uint32 i=0; i<m_items.size(); i++) {
			for(uint32 j=0; j<m_items[i].size(); j++) {

				LayoutItem& layout_item = m_items[i][j];

				//found the cell of the widget
				if(widget == layout_item.GetWidget()) {
					line = i;
					column = j;
					return true;
				}
			}
		}

		return false;
	}

	LayoutItem::CollisionType GridLayout::FindGridLocationAt( int32 xpos, int32 ypos, uint32& line, uint32& column )
	{

		bool collision = false;
		LayoutItem::CollisionType type = LayoutItem::NoCollision;
		uint32 cellRow = 0;
		uint32 cellCol = 0;

		//calculate in which cell to drop the widget
		for(uint32 i=0; i<m_items.size(); i++) {
			for(uint32 j=0; j<m_items[i].size(); j++) {
				LayoutItem& layout_item = m_items[i][j];

				switch(type = layout_item.IsCollision(xpos,ypos,m_panning)) {
					case LayoutItem::NoCollision: break;
					
					default:

						collision = true;
						cellRow = i;
						cellCol = j;
						break;
				}

				if(collision) break;	//else it may crash because we might have changed indexes when adding a widget
			}

			if(collision) break;	//else it may crash because we might have changed indexes when adding a widget
		}
		if(collision) {
			line = cellRow;
			column = cellCol;

			return type;
		}

		return type;	//shouldn't ever happen...
	}

	void LayoutItem::SetWidget( Widget* widget )
	{
		m_widget = widget;
		if(widget) {
			m_shape.SetColor(sf::Color(255,0,0,255));
		} else {
			m_shape.SetColor(sf::Color(255,255,255,0));
		}
	}

	const Rect& LayoutItem::GetRect() const
	{
		return m_rect;
	}

	Widget* LayoutItem::GetWidget() const
	{
		return m_widget;
	}
	//panning doesn't really do anything right now.. use be used for checking collision
	LayoutItem::CollisionType LayoutItem::IsCollision( int x, int y, uint32 panning ) const 
	{
		Rect temp(x,y,1,1);
		if(!gui::IsCollision(temp,m_rect)) {
			return LayoutItem::NoCollision;
		}

		Rect proper = m_rect;
		proper.x += uint32(m_rect.w * 0.1f);		//10% of size reserved
		proper.w -= uint32(2*m_rect.w * 0.1f);
		proper.y += uint32(m_rect.h * 0.1f);
		proper.h -= uint32(2*m_rect.h * 0.1f);

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
		if(x > proper.x + proper.w)
			return LayoutItem::RightCollision;

		return LayoutItem::NoCollision;	//this should never happen
	}

	bool LayoutItem::empty() const
	{
		return (m_widget == NULL);
	}

	void LayoutItem::SetPos( int32 x, int32 y )
	{
		m_rect.SetPos(x,y);
		m_shape.SetPosition(m_rect.GetPos());
	}

	void LayoutItem::SetSize( uint32 width, uint32 height, bool extremity )
	{
		m_rect.SetSize(width,height);
		Rect temp(0,0,width,height);
		if(extremity) {
			temp.x += 2;
			temp.y += 2;
		}
		m_shape = sf::Shape::Rectangle((float)temp.x,(float)temp.y,
					(float)temp.w,(float)temp.h-2,sf::Color(255,255,255,0),2,
					sf::Color(0,0,0,128));

		
		m_shape.SetColor(sf::Color(255,255,255));
	}

	void LayoutItem::SetRect( int32 x, int32 y, uint32 w, uint32 h )
	{
		m_rect.x = x; m_rect.y = y;
		m_rect.w = (int)w; m_rect.h = (int)h;
	}

	void LayoutItem::ResizeWidget( uint32 panning )
	{
		if(!m_widget) return;

		Rect temp = m_rect;
		const sf::Vector2i& sizeHint = m_widget->GetSizeHint();
		temp -= panning;
		
		//resize the widget based on it's size policy
		switch (m_widget->GetHorizontalPolicy())
		{
		case Widget::MinimumExpand:
		case Widget::MaximumExpand:

			switch (m_widget->GetVerticalPolicy())
			{
			case Widget::MinimumExpand:
			case Widget::MaximumExpand:
				m_widget->Resize(temp.w, temp.h);
				break;
			case Widget::Default:
				//get the default behavior for that widget type!
			default: 
				//if the grid decided it can't give you the min required.. resize
				if(temp.h < sizeHint.y) {
					m_widget->Resize(temp.w, temp.h);
				} else { //you have enough space.. get the amount you want
					m_widget->Resize(temp.w, sizeHint.y);
				}
				break;
			}

			break;
		case Widget::Default:
			//get default behavior for that widget type.. if any?
			break;
		default: 
			switch (m_widget->GetVerticalPolicy())
			{
			case Widget::MinimumExpand:
			case Widget::MaximumExpand:
				//if the grid decided it can't give you the min required.. resize
				if(temp.w < sizeHint.x) {
					m_widget->Resize(temp.w, temp.h);
				} else { //you have enough space.. get the amount you want
					m_widget->Resize(sizeHint.x, temp.h);
				}
				break;
			default: 
				//if the grid decided it can't give you the min required.. resize
				if(temp.h < sizeHint.y) 
				{
					if(temp.w < sizeHint.x)
						m_widget->Resize(temp.w, temp.h);
					else m_widget->Resize(sizeHint.x,temp.h);

				} else { //you have enough space.. get the amount you want
					if(temp.w < sizeHint.x)
						m_widget->Resize(temp.w, sizeHint.y);
					else m_widget->Resize(sizeHint.x,sizeHint.y);
				}
				break;
			}
			break;
		}

		//debug
		//m_widget->Resize(temp.w, temp.h);
	}

	bool LayoutItem::IsExpand() const
	{
		return m_expanded;
	}

	void LayoutItem::SetExpand( bool flag )
	{
		m_expanded = flag;
	}

	uint32 LayoutItem::GetRowSpan() const
	{
		return m_rowspan;
	}

	uint32 LayoutItem::GetColSpan() const
	{
		return m_colspan;
	}

	void LayoutItem::SetRowSpan( uint32 count )
	{
		m_rowspan = count;
	}

	void LayoutItem::SetColSpan( uint32 count )
	{
		m_colspan = count;
	}

	LayoutItem::LayoutItem( Widget* widget /*= NULL*/,uint32 rowspan/*=1*/, 
							uint32 colspan/*=1*/ )
	{
		m_widget = widget;
		m_expanded = false;
		m_rowspan = rowspan;
		m_colspan = colspan;
	}

	void LayoutItem::Draw( sf::RenderWindow& window ) const
	{
 		if(m_expanded) 
 			return;

		window.Draw(m_shape);
	}

	void LayoutItem::UpdateWidgetPos( uint32 panning )
	{
		if(!m_widget) return;

		Rect temp = m_rect;
		Rect rect = m_widget->GetRect();
		temp -= panning;

		uint32 xpos(0), ypos(0);

		rect = m_widget->GetRect();

		//center the widget rect
		xpos = m_rect.x + m_rect.w/2 - rect.w/2;
		ypos = m_rect.y + m_rect.h/2 - rect.h/2;

		//set it's position based on it's size policy
		m_widget->SetPos(xpos, ypos,true);

	}

	bool GridLayout::IsExtremity(uint32 row, uint32 col) const
	{
		if((col == 0 || col == (m_items[row].size()-1)) ||
			(row == 0 || row == (m_items.size()-1))) 
		{
			return true;
		}
		return false;
	}

	bool GridLayout::IsCollision( const Rect& rect ) const
	{
		if(!s_gui->IsEditEnabled()) {
			//normally you don't care about events inside the grid
			//only if inside a child of the grids..unless in edit mode
			for(WidgetList::reverse_iterator it = m_widgets.rbegin();
					it != m_widgets.rend(); it++) 
			{
				if(it->second->IsCollision(rect)) {
					return true;
				}
			}
			return false;
		} else {
			return Widget::IsCollision(rect);
		}
	}

	void GridLayout::SetParent( Widget* parent )
	{
		//stop listening to the old parent's events
		if(m_parent) {
			m_mediator.Disconnect(m_parent,"default",events::OnResize);
		}

		Widget::SetParent(parent);
		
		if(!parent) return;

		if(GetVerticalPolicy()	!= Widget::Fixed   ||
		   GetHorizontalPolicy()!= Widget::Fixed)
		{
			m_mediator.Connect(parent,"default",events::OnResize,false);
		}
		ResizeFromParent();
	}

	void GridLayout::Update( float diff )
	{
		Widget::Update(diff);
			
		Event* e = NULL;
		while(e = m_mediator.GetEvent()) {
			if(e->GetType() != events::OnResize) {
				error_log("Received unsupported event type: %u", e->GetType());
				continue;
			}
			gui::OnResize* ev = (gui::OnResize*)e;
			Widget* parent = ev->GetWidget();
			Rect new_rect;
			if(parent == m_parent) {
				const Rect& prect = parent->GetRect();
				const Rect& oldRect = ev->GetOldRect();
				ResizeFromParent(oldRect);	
			}
		}
	}

	void GridLayout::ResizeFromParent( Rect oldRect /*= Rect(0,0,0,0)*/ )
	{
		if(!m_parent) {
			return;
		}
		Rect new_rect;
		const Rect& prect = m_parent->GetRect();

		if(!oldRect) {
			oldRect = prect;
		}

		switch(GetHorizontalPolicy())
		{
		case Widget::MinimumExpand:
		case Widget::MaximumExpand:
			new_rect.w = prect.w;
			new_rect.x = m_rect.x;	//don't move it
			new_rect.y = m_rect.y;
			break;
		case Widget::ScaledExpand:	
			{
				float scale = m_rect.w / (float) oldRect.w;
				float posXScale = m_rect.x / (float) oldRect.w;
				float posYScale = m_rect.y / (float) oldRect.h;

				new_rect.x = uint32(new_rect.w * posXScale);
				new_rect.y = uint32(new_rect.h * posYScale);

				new_rect.w = uint32(prect.w * scale);
			}
			break;

		}

		switch(GetVerticalPolicy())
		{
		case Widget::MinimumExpand:
		case Widget::MaximumExpand:
			new_rect.h = prect.h;

			new_rect.x = m_rect.x;	//don't move it
			new_rect.y = m_rect.y;
			break;
		case Widget::ScaledExpand:	
			{
				float scale = m_rect.h / (float) oldRect.h;
				float posXScale = m_rect.x / (float) oldRect.w;
				float posYScale = m_rect.y / (float) oldRect.h;

				new_rect.x = uint32(new_rect.w * posXScale);
				new_rect.y = uint32(new_rect.h * posYScale);

				new_rect.h = uint32(prect.h * scale);
			}
			break;

		}
		Resize(new_rect.w, new_rect.h,true);
		SetPos(new_rect.x, new_rect.y,true);
	}

	void GridLayout::Resize( int w, int h,bool save /* = true */ )
	{
		Widget::Resize(w,h,save);

		ComputeCells();
	}

}

