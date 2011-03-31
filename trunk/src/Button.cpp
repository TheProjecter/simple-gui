#include "../include/gui/Button.hpp"
#include "../include/gui/GuiManager.hpp"
#include <sstream>

namespace gui
{
	void Button::SetActiveImage()
	{
		sf::Color temp = m_shape.GetColor();

		//make the shape lighter by ~20% ?
		temp.r = temp.r < 255- 50 ? temp.r + 50 : 255;
		temp.g = temp.g < 255- 50 ? temp.g + 50 : 255;
		temp.b = temp.b < 255- 50 ? temp.b + 50 : 255;

		m_shape.SetColor(temp);
	}
	void Button::SetInactiveImage()
	{
		sf::Color temp = m_shape.GetColor();
		
		//make the shape darker by ~20% ?
		temp.r = temp.r > 50 ? temp.r - 50 : 0;
		temp.g = temp.g > 50 ? temp.g - 50 : 0;
		temp.b = temp.b > 50 ? temp.b - 50 : 0;

		m_shape.SetColor(temp);
	}
	void Button::Draw() const
	{
		if(!m_visible || !s_gui->GetWindow()) return;
		Widget::Draw();
		s_gui->GetWindow()->Draw(m_text);
	}

	void Button::OnClickPressed(sf::Event* event)
	{
		Widget::OnClickPressed(event);
		
		//it was clicked
		m_active = false;
		SetInactiveImage();
	}
	void Button::OnClickReleased(sf::Event* event)
	{
		sf::Vector2f pos;
		pos.x = (float)event->MouseButton.X;
		pos.y = (float)event->MouseButton.Y;
		ConvertCoords(pos);
		if(IsCollision(Rect((int)pos.x, (int)pos.y,1,1))) {
			Widget::OnClickReleased(event);
		}

		//it was released
		m_active = true;
		SetActiveImage();

	}

	Button::Button() : m_active(true)
	{
		SetPos(0,0); 
		m_text.SetSize(16); 
		m_movable = false; 
		m_type = BUTTON;
	}

	void Button::SetPos( int x, int y, bool forceMove /*= false */)
	{
		Widget::SetPos(x,y,forceMove);
		std::stringstream s;

		m_text.SetPosition(m_rect.x+m_rect.w/2-m_text.GetRect().GetWidth()/2,
			m_rect.y+m_rect.h/2-m_text.GetRect().GetHeight()/2);
	}

	void Button::SetText( const std::string& text )
	{
		m_text.SetText(text);
	}

	void Button::InitGraphics()
	{
		if(m_settings.HasInt32Value("button-color")) {
			m_shape.SetColor(UnsignedToColor(m_settings.GetInt32Value("button-color")));
			m_individualTheme = true;
		} else if(s_gui->GetTheme()) {
			m_shape.SetColor(s_gui->GetTheme()->GetColor("button-color"));
		}
	}

	ImageButton::ImageButton()
	{
		m_type = IMAGE_BUTTON;
		m_text.SetSize(16);
		m_movable = false;
		m_sprite = new sf::Sprite;

		sf::Image* img = s_gui->GetTheme()->GetImage("button-image");
		if(!img) {
			debug_log("Couldn't load image for button %s", m_name.c_str());
		} else {
			m_sprite->SetImage(*img);
		}
	}

	void ImageButton::Draw() const
	{
		if(!m_sprite || !s_gui->GetWindow() || !m_visible) return;

		s_gui->GetWindow()->Draw(*m_sprite);
		s_gui->GetWindow()->Draw(m_text);

		//buttons should really have children in the first place :|
// 		for(WidgetList::iterator it = m_widgets.begin(); 
// 			it != m_widgets.end(); it++) 
// 		{
// 			it->second->Draw();
// 		}
	}

	void ImageButton::SetInactiveImage()
	{
		if(!m_sprite) return;

		m_sprite->SetColor(sf::Color(200,200,200));
	}

	void ImageButton::SetActiveImage()
	{
		if(!m_sprite) return;

		m_sprite->SetColor(sf::Color(255,255,255));
	}

	void ImageButton::InitGraphics()
	{
		sf::Image* img = s_gui->GetTheme()->GetImage("button-image");
		if(!img) {
			debug_log("Couldn't load image for button %s", m_name.c_str());
		} else {
			m_sprite->SetImage(*img);
		}
	}
}


