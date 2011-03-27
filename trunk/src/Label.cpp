#include "../include/gui/Label.hpp"
#include "../include/gui/GuiManager.hpp"


namespace gui {

	void Label::Draw() const
	{
		Widget::Draw();

		//then draw the text
		if(s_gui->GetWindow())
			s_gui->GetWindow()->Draw(text);
	}
	void Label::SetPos(int x, int y)
	{
		m_rect.x = x; m_rect.y = y;  
		switch(alignment) {
			case Left:	
				text.SetPosition((float)x+5, (float)y+5);			
				break;
			case Right:	
				text.SetPosition((float)x+m_rect.w-5-text.GetRect().GetWidth(), (float)y+5);	
				break;
			case Top:	text.SetPosition((float)x+5, (float)y+5);			
				break;	
			case Bottom:text.SetPosition((float)x+5, (float)y+m_rect.h-5);	
				break;
			case CenterHorizontal:	
				text.SetPosition((float)x+m_rect.w/2-text.GetRect().GetWidth()/2,(float)y+5); 
				break;
			case CenterVertical:	
				text.SetPosition((float)x+5, (float)y+m_rect.h/2-text.GetRect().GetHeight()/2); 
				break;
			case Center:			
				text.SetPosition((float)x+m_rect.w/2-text.GetRect().GetWidth()/2,
								(float)y+m_rect.h/2-text.GetRect().GetHeight()/2); 
				break;
			case LeftBottom:		
				text.SetPosition((float)x+5, (float)y+m_rect.h-5-text.GetRect().GetWidth()); 
				break;
			case LeftTop:			
				text.SetPosition((float)x+5, (float)y+5);
				break;
			case RightBottom:		
				text.SetPosition((float)x+m_rect.w-5-text.GetRect().GetWidth(),
					y+m_rect.w-5-text.GetRect().GetHeight());
				break;
			case RightTop:			
				text.SetPosition((float)x+5,(float)y+5); 
				break;
			default: break;
		}
	}
	void Label::Update(float diff)
	{
		if(!m_visible) return;
		Widget::Update(diff);
		if(text.GetRect().GetWidth()>m_rect.w) 
			m_rect.w = (int)text.GetRect().GetWidth(); 
		if(text.GetRect().GetHeight() > m_rect.h)
			m_rect.h = (int)text.GetRect().GetHeight();
	}

	Label::Label() : alignment(Center)
	{
		m_type = LABEL;
	}

	void Label::SetAlignment( Alignment a )
	{
		alignment = a;
	}
}

