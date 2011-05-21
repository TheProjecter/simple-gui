#include "../include/gui/LineEdit.hpp"
#include "../include/gui/Debug.hpp"
#include "../include/gui/GuiManager.hpp"

#include <iostream>


namespace gui 
{
	LineEdit::LineEdit(): m_cursorIndex(0),m_visibleChars(0),
						m_cursorStartIndex(0),m_allowedChars(255),
						m_cursorDiff(0), m_cursorShow(true),m_isPassword(false)
	{
		m_type = LINE_EDIT;
		m_cursor = sf::Shape::Line(0,0,0,(float)m_rect.h,5,sf::Color(255,0,0));

		m_visibleText.SetSize(14);	//hardcoded value..
		m_visibleText.SetColor(sf::Color(0,0,0));

		//buttons particular size hint
		m_sizeHint.x = 75;
		m_sizeHint.y = 25;

		//buttons particular size policy
		m_horizontalPolicy	= MaximumExpand;
		m_verticalPolicy	= Fixed;	
	}

	void LineEdit::Draw() const
	{
		Widget::Draw();
		if(!m_visible) return;
		s_gui->GetWindow().Draw(m_visibleText);

		//make the cursor blink every 300 ms
		if(m_cursorDiff > 300) {
			m_cursorDiff = 0;
			m_cursorShow = !m_cursorShow;
		}
		if(m_cursorShow && IsFocus())
			s_gui->GetWindow().Draw(m_cursor);
		
	}
/*
	void LineEdit::SetDefaultImage()
	{
		sf::Image img;
		img.Create(100,50,sf::Color(255,255,255));
		sprite.SetImage(ImageManager::GetSingleton().GetImage(img,"basic_line_edit"));
		pos.Setup(0,0,sprite.GetImage()->GetWidth(),sprite.GetImage()->GetHeight());
		sprite.SetPosition(0,0);
	}
*/

	void LineEdit::SetPos(int x, int y,bool forceMove /*= false*/, bool save/*=true*/)
	{	
		Widget::SetPos(x,y,forceMove,save);
		if(!m_movable && !forceMove) return;

		m_visibleText.SetPosition((float)m_rect.x,(float)m_rect.y);
		_SetCursorPos();
	}

	void LineEdit::OnKeyPressed(sf::Event *event)
	{
		bool changed = false;
		if(event->Key.Code == sf::Key::Back) {
		} else if(event->Key.Code == sf::Key::Delete) {
		
			//erase the character on the right
			if(m_cursorIndex < m_totalText.size()) {
				m_totalText.erase(m_cursorIndex,1); 

				TestSizeErrors();
				SetVisibleText();
				_SetCursorPos();
			}
			return;
		} else if(event->Key.Code == sf::Key::Right) {
			if(m_cursorIndex >= m_totalText.size()) return;

			const sf::Input& input = s_gui->GetWindow().GetInput();
			if(input.IsKeyDown(sf::Key::LControl) ||
			   input.IsKeyDown(sf::Key::RControl)) 
			{
				//calculate the displacement
				int temp = m_cursorIndex;
				m_cursorIndex = _FindCursorPos(m_totalText,m_cursorIndex,1);
				temp = m_cursorIndex - temp;
				if(m_cursorIndex > m_cursorStartIndex+m_visibleChars) {
					m_cursorStartIndex += temp;
					TestSizeErrors(true);
				} else TestSizeErrors();
			} else if(m_cursorIndex < (m_cursorStartIndex + m_visibleChars) ) {
				m_cursorIndex++;
				TestSizeErrors();
			} else {
				if(m_cursorIndex < m_totalText.size()) {
					m_cursorStartIndex++;
					m_cursorIndex++;
					TestSizeErrors(true);
				}
			}
			SetVisibleText();
			_SetCursorPos();
			return;
		

		} else if(event->Key.Code == sf::Key::Left) {

			const sf::Input& input = s_gui->GetWindow().GetInput();
			if(input.IsKeyDown(sf::Key::LControl) ||
				input.IsKeyDown(sf::Key::RControl)) {
					m_cursorIndex = _FindCursorPos(m_totalText,m_cursorIndex,-1);
					m_cursorStartIndex = 0;
					m_visibleChars = m_totalText.size();
			} else if(m_cursorIndex > 0 && m_totalText.size()) {
				if(m_cursorIndex > m_cursorStartIndex) {
					m_cursorIndex--;						
				} else {
					m_cursorIndex--;
					if(m_cursorStartIndex>0) {
						m_cursorStartIndex--;
					}
				}
				
			}
			changed = true;
			

		} else if(event->Key.Code == sf::Key::Home){
			m_cursorStartIndex = 0;
			m_cursorIndex = 0;
			changed = true;
		} else if(event->Key.Code == sf::Key::End) {
			m_cursorIndex = m_totalText.size();
			m_cursorStartIndex = m_cursorIndex - m_visibleChars;
			TestSizeErrors(true);
			SetVisibleText();
			_SetCursorPos();
			return;
		}
		if(changed) {
			//reset the visible text, as it might have been modified
			TestSizeErrors();
			SetVisibleText();
			_SetCursorPos();
		}
	}

	void LineEdit::OnTextEntered(sf::Event* event)
	{
		//backspace pressed ?
		if(event->Text.Unicode == 8) {
			debug_log("Backspace pressed!");

			//erase the character on left
			if(m_cursorIndex > 0) {
				m_totalText.erase(--m_cursorIndex,1); 
				if(m_cursorIndex <= m_cursorStartIndex) {
					if(m_cursorStartIndex > 0) m_cursorStartIndex--;
				} else --m_visibleChars;
				SetVisibleText();
				TestSizeErrors();
				_SetCursorPos();
			}
			return;
		}

		//max character number has been reached
		if(m_totalText.size() >= m_allowedChars) return; 

		m_totalText.insert(m_cursorIndex++,1,event->Text.Unicode);
		
		//check whether it fits in the line edit box
		if(m_isPassword){
			std::string temp;
			temp.insert(0,m_visibleChars+1, '*');
			m_visibleText.SetText(temp);
		} else m_visibleText.SetText(m_totalText.substr(m_cursorStartIndex,m_visibleChars+1));

		TestSizeErrors(true);
		SetVisibleText();
		_SetCursorPos();
	}

	void LineEdit::SetBackgroundColor(sf::Color color)
	{
		Widget::SetBackgroundColor(color);
		sf::Vector2f p1,p2;
		p1 = p2 = GetPos();
		p2.y += m_rect.h;
		m_cursor = sf::Shape::Line(p1,p2,3.f,sf::Color(0,0,0));
	}

	void LineEdit::_SetCursorPos()
	{
		sf::Vector2f pos;
		sf::String s;

		s.SetFont(m_visibleText.GetFont());
		std::string temp;
		if(m_isPassword) {
			temp.insert(0,m_cursorIndex-m_cursorStartIndex, '*');
		} else {
			temp = m_totalText.substr(m_cursorStartIndex,m_cursorIndex-m_cursorStartIndex);
		}
		s.SetText(temp);
		pos.x = m_rect.x + s.GetRect().GetWidth();
		pos.y = (float)m_rect.y;

		m_cursor.SetPosition(pos);
	}

	void LineEdit::Update( float diff )
	{
		Widget::Update(diff);
		m_cursorDiff += diff;
	}

	void LineEdit::SetVisibleText()
	{
		if(m_isPassword) {
			std::string temp;
			temp.insert(0,m_visibleChars, '*');
			m_visibleText.SetText(temp);
		} else {
			m_visibleText.SetText(m_totalText.substr(m_cursorStartIndex,m_visibleChars));
		}
	}

	void LineEdit::SetPasswordField( bool flag )
	{
		m_isPassword = flag;
	}

	void LineEdit::TestSizeErrors( bool moveStartIndex /*= false*/ )
	{
		sf::String tmp;
		sf::String tmp1;
		std::string temp;
		tmp.SetFont(m_visibleText.GetFont());
		tmp.SetSize(m_visibleText.GetSize());
		tmp.SetStyle(m_visibleText.GetStyle());

		tmp1.SetFont(m_visibleText.GetFont());
		tmp1.SetSize(m_visibleText.GetSize());
		tmp1.SetStyle(m_visibleText.GetStyle());

		if(moveStartIndex) {
			bool longWord = false;
			int count = -1; 
			tmp.SetText(m_totalText.substr(m_cursorStartIndex,m_visibleChars+1));
			while(count < (int)m_visibleChars) {	//free 2 chars max to have space for 1.. should suffice
				if(tmp.GetRect().GetWidth() > m_rect.w) {
					count++;
					longWord = true;
					temp = m_totalText.substr(++m_cursorStartIndex,m_visibleChars);
					tmp.SetText(temp);
				} else break;
			} 
			if(!longWord) {
				++m_visibleChars;
			} else {
				m_visibleChars -= count;
			}
		}
		temp.clear();

		//initiate the two strings, the second one with +1 size if possible
		if(m_isPassword) {
			temp.insert(0,m_visibleChars, '*');
		} else {
			temp = m_totalText.substr(m_cursorStartIndex,m_visibleChars);
		}
		tmp.SetText(temp);
		tmp1.SetText(temp);
	
		//these are to avoid infinite loops in some cases
		uint32 loopCount = 0;
		uint32 visibleChars = m_visibleChars;

 		//size error checking
 		while(true) 
		{
			//if the text fits the rect
			if(tmp.GetRect().GetWidth() <= m_rect.w && tmp1.GetRect().GetWidth() >= m_rect.w) break;
		
			loopCount++;

			//if it's smaller than it should add +1 to tmp and +2 to tmp1
			if(tmp.GetRect().GetWidth() < m_rect.w) {
				if(m_isPassword) { 
					temp.clear(); 
					temp.insert(0,m_visibleChars,'*'); 
				} else {
					temp = m_totalText.substr(m_cursorStartIndex,m_visibleChars);
				}
				tmp.SetText(temp);

				if(m_isPassword)  { 
					temp.clear(); 
					if(m_cursorStartIndex + m_visibleChars /*+ 1*/ < m_totalText.size())
						m_visibleChars++;
					else break;	//don't go on if there are no more characters

					temp.insert(0,m_visibleChars,'*'); 
				} else { 
					if(m_cursorStartIndex + m_visibleChars /*+ 1*/ < m_totalText.size())
						m_visibleChars++;
					else break;	//don't go on if there are no more characters

					temp = m_totalText.substr(m_cursorStartIndex,m_visibleChars);
				}
				tmp1.SetText(temp);
			} else {
				if(m_isPassword) { 
					temp.clear(); 
					temp.insert(0,m_visibleChars,'*'); 
				} else {
					temp = m_totalText.substr(m_cursorStartIndex,m_visibleChars);
				}
				tmp1.SetText(temp);

				if(m_isPassword)  { 
					temp.clear(); 
					if(m_visibleChars > 0)
						m_visibleChars--;
					else break;	//don't go on if there are no more characters

					temp.insert(0,m_visibleChars,'*'); 
				} else { 
					if(m_visibleChars > 0)
						m_visibleChars--;
					else break;	//don't go on if there are no more characters

					temp = m_totalText.substr(m_cursorStartIndex,m_visibleChars);
				}
				tmp.SetText(temp);
			}
			//if you looped and got the same result.. then it's an infinite loop
			if(loopCount > 4 && m_visibleChars == visibleChars) break;	//TODO: this won't work..
		}
		
		temp = tmp.GetText();
		m_visibleChars = temp.size();
	}

	//used when clicking ctrl+left/right, searches the appropriate pos for the cursor
	gui::uint32 LineEdit::_FindCursorPos(const std::string& text, uint32 pos,int searchType)
	{
		uint32 i = 0;
		if(searchType >= 0) {
			bool charFound = false;
			for(i=pos; i<text.size();i++) {
				if(isalpha(text[i])) charFound = true;
				//if you found a whitespace and you found a char before that then you can stop
				else if(text[i] == ' ' && charFound) {
					i++;	//go back to the last character visited
					break; 		
				}
			}
		} else {
			if(pos > 0) pos--;
			bool charFound = false;
			for(i=pos; i>0;i--) {
				if(isalpha(text[i])) charFound = true;
				//if you found a whitespace and you found a char before that then you can stop
				else if(text[i] == ' ' && charFound) {
					i++;	//go back to the last character visited
					break; 		
				}
			}
		}
		return i;
	}

	void LineEdit::InitGraphics()
	{
		if(m_settings.HasUint32Value("line-edit_bg_color")) {
			m_shape.SetColor(UnsignedToColor(m_settings.GetUint32Value("background-color")));
			m_cursor.SetColor(sf::Color(0,0,0));
			m_individualTheme = true;
		} else if(s_gui->GetTheme()){
			m_cursor.SetColor(sf::Color(0,0,0));
			m_shape.SetColor(s_gui->GetTheme()->GetColor("line-edit_bg_color"));
		}		
	}
}