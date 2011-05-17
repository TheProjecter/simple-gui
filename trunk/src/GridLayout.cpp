#include "../include/gui/GridLayout.hpp"
#include "../include/gui/GuiManager.hpp"
#include <sstream>

namespace gui
{
	GridLayout::GridLayout()
	{
		m_type = GRID_LAYOUT;
		m_panning = 5;		//by default 2 pixels panning

		//resize to a 2 by 2 grid
		m_items.resize(2);
		for(uint32 i=0; i<m_items.size(); i++) {
			m_items[i].resize(2);
		}

		ComputeCells();
	}

	GridLayout::~GridLayout()
	{
		//nothing to do really...
	}
	bool GridLayout::AddWidgetToGrid( Widget* widget, uint32 line, uint32 column )
	{
		assert(widget && m_items.size() > line && m_items[line].size() > column);

		LayoutItem& layout_item = m_items[line][column];
		if(!layout_item.empty() || layout_item.IsExpand()) {
			return false;
		}

		Widget::AddWidgetForced(widget);
		m_items[line][column].SetWidget(widget);	
		

		//adding widgets can also lead to useless redundant rows/columns!
		RemoveEmptyColumnAndLines();	
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
			float coeff = m_rect.w / (float) columnWidth;	//resize all columns by coeff to fit my size

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
			float coeff = m_rect.w / (float) rowHeight;	//resize all columns by coeff to fit my size

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

				//make sure you have the correct width for row-spanned items
				for(uint32 k=j; k<j+colspan; k++) {
					width += columnsInfo[k].first;
				}
				uint32 height = 0;

				//make sure you have the correct height for row-spanned items
				for(uint32 k=i; k<i+rowspan; k++) {
					height += rowsInfo[k].first;
				}
				//check if the cell is an outer-cell(extremity)
				bool extremity = false;
				if((j == 0 || j == (m_items[i].size()-1)) ||
					(i == 0 || i == (m_items.size()-1))) 
				{
					extremity = true;
				}

				layout_item.SetSize(width,height,extremity);
				layout_item.SetPos(xpos,ypos);

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

				layout_item.UpdateWidgetPos(m_panning);
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
		if(!Widget::AddWidget(child)) return false;

		SaveGridProperties();
		return true;
	}

	void GridLayout::SaveGridProperties()
	{
		if(!m_items.size()) {
			error_log("Grid is empty!");
			return;
		}

		std::stringstream s;
		
		//the size of the grid.. assume every columns has the same size
		s << m_items.size() << " " << m_items[0].size() << " ";
		for(uint32 i=0; i<m_items.size(); i++) {
			for(uint32 j=0; j<m_items[i].size(); j++) {
				LayoutItem& layout_item = m_items[i][j];
				s << layout_item.GetRowSpan() << " " << layout_item.GetColSpan() << " ";
			}
		}
		m_settings.SetStringValue("grid-properties", s.str());
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

		for(uint32 i=0; i<rowsize; i++) {
			for(uint32 j=0; j<colsize; j++) {
				uint32 rowspan = 0;
				uint32 colspan = 0;

				s >> rowspan >> colspan;
				LayoutItem& layout_item = m_items[i][j];
				layout_item.SetRowSpan(rowspan);
				layout_item.SetColSpan(colspan);
			}
		}
		for(uint32 i=0; i<rowsize; i++) {
			for(uint32 j=0; j<colsize; j++) {
				LayoutItem& layout_item = m_items[i][j];		

				uint32 rowspan = layout_item.GetRowSpan();
				uint32 colspan = layout_item.GetColSpan();

				for(uint32 k=j+1; k<j+colspan; k++) {
					m_items[i][k].SetExpand(true);
				}
				for(uint32 k=i+1; k<i+rowspan; k++) {
					m_items[k][j].SetExpand(true);
				}
				
			}
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
			if(!m_items[line][j].empty()) {
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
			if(!m_items[i][column].empty()) {
				return false;
			}
		}

		return true;	
	}

	bool GridLayout::RemoveLineIfEmpty( uint32 line )
	{
		if(!IsLineEmpty(line)) 
			return false;

		//don't delete anything if the grid is smaller than 2x2
		if(m_items.size() <= 2) 
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

		//don't delete anything if the grid is smaller than 2x2
		if(m_items[0].size() <= 2) 
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
				drag->GetTarget()->Resize(width, height);

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
			if(!FindGridLocationAt((uint32)drag->GetCurrentMousePos().x, (uint32)drag->GetCurrentMousePos().y, line, column)) {
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
		if(!FindGridLocationAt((uint32)drag->GetCurrentMousePos().x, (uint32)drag->GetCurrentMousePos().y, line, column)) {
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
					(float)temp.w,(float)temp.h-2,sf::Color(0,0,0,0),2,
					sf::Color(0,255,0));

		m_shape.SetColor(sf::Color(0,0,0,128));
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
		Rect rect = m_widget->GetRect();
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
			default: 
				m_widget->Resize(temp.w, rect.h);
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
				m_widget->Resize(rect.w, temp.h);
			default: 
				m_widget->Resize(rect.w, rect.h);
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

	LayoutItem::LayoutItem( Widget* widget /*= NULL*/, bool expanded /*= false*/ )
	{
		m_widget = widget;
		m_expanded = expanded;
		m_rowspan = 1;
		m_colspan = 1;
	}

	void LayoutItem::Draw( sf::RenderWindow& window ) const
	{
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
}