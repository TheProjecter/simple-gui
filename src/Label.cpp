#include "../include/gui/Label.hpp"
#include "../include/gui/GuiManager.hpp"


namespace gui {

	void Label::Draw() const
	{
		Widget::Draw();

		//then draw the text
		s_gui->GetWindow().Draw(m_text);
	}
	void Label::SetPos(int x, int y, bool forceMove /*=false*/, bool save /*=true*/)
	{
		Widget::SetPos(x,y,forceMove,save);

		switch(alignment) {
			case Left:	
				m_text.SetPosition((float)x+5, (float)y+5);			
				break;
			case Right:	
				m_text.SetPosition((float)x+m_rect.w-5-m_text.GetRect().GetWidth(), (float)y+5);	
				break;
			case Top:	m_text.SetPosition((float)x+5, (float)y+5);			
				break;	
			case Bottom:m_text.SetPosition((float)x+5, (float)y+m_rect.h-5);	
				break;
			case CenterHorizontal:	
				m_text.SetPosition((float)x+m_rect.w/2-m_text.GetRect().GetWidth()/2,(float)y+5); 
				break;
			case CenterVertical:	
				m_text.SetPosition((float)x+5, (float)y+m_rect.h/2-m_text.GetRect().GetHeight()/2); 
				break;
			case Center:			
				m_text.SetPosition((float)x+m_rect.w/2-m_text.GetRect().GetWidth()/2,
								(float)y+m_rect.h/2-m_text.GetRect().GetHeight()/2); 
				break;
			case LeftBottom:		
				m_text.SetPosition((float)x+5, (float)y+m_rect.h-5-m_text.GetRect().GetWidth()); 
				break;
			case LeftTop:			
				m_text.SetPosition((float)x+5, (float)y+5);
				break;
			case RightBottom:		
				m_text.SetPosition((float)x+m_rect.w-5-m_text.GetRect().GetWidth(),
					y+m_rect.w-5-m_text.GetRect().GetHeight());
				break;
			case RightTop:			
				m_text.SetPosition((float)x+5,(float)y+5); 
				break;
			default: break;
		}
	}
	void Label::Update(float diff)
	{
		if(!m_visible) return;
		Widget::Update(diff);
		if(m_text.GetRect().GetWidth()>m_rect.w) 
			m_rect.w = (int)m_text.GetRect().GetWidth(); 
		if(m_text.GetRect().GetHeight() > m_rect.h)
			m_rect.h = (int)m_text.GetRect().GetHeight();
	}

	Label::Label() : alignment(Center)
	{
		m_type = LABEL;


		//buttons particular size hint
		m_sizeHint.x = 75;
		m_sizeHint.y = 25;

		//buttons particular size policy
		m_horizontalPolicy	= MinimumExpand;
		m_verticalPolicy	= Fixed;	
	}

	void Label::SetAlignment( Alignment a )
	{
		alignment = a;
	}

	void Label::ReloadSettings()
	{
		Widget::ReloadSettings();

		if(m_settings.HasStringValue("text")) {
			std::string text = m_settings.GetStringValue("text");
			SetText(text);
		}
		if(m_settings.HasUint32Value("size")) {
			uint32 size = m_settings.GetUint32Value("size");
			m_text.SetSize(size);
		}
	}

	void Label::SetText( const std::string& text )
	{
		m_text.SetText(text);
	}

	std::string Label::GetText() const
	{
		return m_text.GetText();
	}
}

