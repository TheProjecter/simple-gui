#include "../include/gui/Slider.hpp"
#include "../include/gui/GuiManager.hpp"

namespace gui 
{
	Slider::Slider() : m_curValue(0),m_lowerLimit(0), m_upperLimit(100), 
					   m_sliderDrag(false),m_hotSpotX(0),m_hotSpotY(0),m_cursor(NULL)
	{
		m_movable = false; 
		m_type = SLIDER;
	}

	Slider::Slider( int upper, int lower ) : m_curValue(0),m_lowerLimit(lower),
						m_upperLimit(upper), m_sliderDrag(false),m_hotSpotX(0),
						m_hotSpotY(0),m_cursor(NULL)
	{
		m_movable = false;
		m_type = SLIDER;
		SetLimits(upper,lower);
	}
	int Slider::GetValue()
	{
		return m_curValue;
	}

	void Slider::SetValue( int value )
	{
		m_curValue = value;
	}

	void Slider::SetLimits( int lower, int upper )
	{
		if(lower < upper) { 
			m_upperLimit = upper; 
			m_lowerLimit = lower; 
		} else {
			m_upperLimit = lower; 
			m_lowerLimit = upper; 
		}
	}

/*
	void Slider::SetDefaultImage()
	{
		sprite.SetImage(ImageManager::GetSingleton().GetImage("assets/gfx/slider.png","default_slider"));
		cursor.SetImage(ImageManager::GetSingleton().GetImage("assets/gfx/slider-cursor.png", "default_slider_cursor"));

		pos.x = (int)sprite.GetPosition().x; pos.y = (int)sprite.GetPosition().y;
		pos.w = (int)sprite.GetSize().x;	 pos.h = (int)sprite.GetSize().y;
	}
*/

	void Slider::OnClickPressed(sf::Event *event)
	{
		m_needUpdate = true;

		sf::Vector2f pos((float)event->MouseButton.X,
						 (float)event->MouseButton.Y);
		ConvertCoords(pos);
		if(gui::IsCollision(Rect(pos,sf::Vector2f(1,1)),m_cursorRect))
		{

			//start drag
			m_sliderDrag = true;

			sf::Vector2f spot((float)event->MouseButton.X,(float)event->MouseButton.Y);
			ConvertCoords(pos);

			//set the hotspot (relative to the position of the cursor)
			m_hotSpotX = (int)spot.x - m_cursorRect.x;
			m_hotSpotY = (int)spot.y - m_cursorRect.y;
		}
		//if the click location wasn't on the cursor..
		else {
			sf::Vector2f spot((float)event->MouseButton.X,(float)event->MouseButton.Y);
			ConvertCoords(pos);

			if((int)spot.x > m_cursorRect.x) {
				m_curValue += (m_upperLimit-m_lowerLimit)/10;
				if(m_curValue > m_upperLimit)
					m_curValue = m_upperLimit;
			} else {
				m_curValue -= (m_upperLimit-m_lowerLimit)/10;
				if(m_curValue < m_lowerLimit) 
					m_curValue = m_lowerLimit;
			}
		}
	}

	void Slider::Update(float diff)
	{

		Widget::Update(diff);
		Widget::Draw(m_cursor);
	}

	void Slider::Draw() const
	{
		Widget::Draw();
		if(!m_cursor || !s_gui->GetWindow()) return;

		sf::Sprite sprite;
		sprite.SetImage(*m_cursor);
		sprite.SetPosition(sf::Vector2f((float)m_cursorRect.x,(float)m_cursorRect.y));
		sprite.Resize(sf::Vector2f((float)m_cursorRect.w,(float)m_cursorRect.h));
		sprite.SetColor(sf::Color(255,255,255,m_transparency));

		s_gui->GetWindow()->Draw(sprite);
	}
	void Slider::OnClickReleased(sf::Event *event)
	{
		m_sliderDrag = false;
		CalculateSliderPos();
	}

	void Slider::OnOtherEvents(sf::Event* event)
	{
		if(event->Type == sf::Event::MouseMoved) 
		{
			int mouseX = event->MouseMove.X; 
			int mouseY = event->MouseMove.Y;
			sf::Vector2f mouse((float)mouseX, (float)mouseY);
			ConvertCoords(mouse);

			mouseX = (int)mouse.x; 
			mouseY = (int)mouse.y;

			if(!m_sliderDrag) return;

			//calculate where the cursor would land
			int landSpotX = mouseX - m_hotSpotX;

			//if it lands out of bonds.. correct that !
			if(landSpotX > m_rect.x + m_rect.w - m_cursorRect.w) {
				landSpotX = m_rect.x + m_rect.w -m_cursorRect.w;
				m_curValue = m_upperLimit;
			} else if(landSpotX < m_rect.x) {
				landSpotX = m_rect.x;
				m_curValue = m_lowerLimit;
			} else {
				double percent = (float)((landSpotX-m_rect.x)/(float)(m_rect.w-m_cursorRect.w));
				int maxNumbers = m_upperLimit - m_lowerLimit;

				m_curValue = m_lowerLimit + int(maxNumbers*percent);
			}



			//now sets the position of the cursor
			m_cursorRect.x = landSpotX;

			CalculateSliderPos();
		}
	}

	void Slider::Resize(int w, int h)
	{
		m_needUpdate = true;

		m_rect.w = w; m_rect.h = h;
		m_cursorRect.w = w/15;
		m_cursorRect.h = h;
		if(m_cursorRect.w == 0) 
			m_cursorRect.w = 1;
	}

	void Slider::CalculateSliderPos()
	{
		if(m_curValue <= m_lowerLimit) m_curValue = m_lowerLimit;
		else if(m_curValue >= m_upperLimit) m_curValue = m_upperLimit;

		//update the cursor position
		int xAxisDisplacement = std::abs(m_curValue - m_lowerLimit);  
		int maxNumbersPosible = m_upperLimit - m_lowerLimit;
		double percent = xAxisDisplacement/(double)maxNumbersPosible;

		//calculate position for the cursor
		int w = m_rect.w - m_cursorRect.w;
		int x = m_rect.x; 
		int y = m_rect.y;

		//calculate the x-axis displacement
		int xDisp = int(percent*w);

		//move the cursor
		m_cursorRect.x = m_rect.x + xDisp;
		m_cursorRect.y = m_rect.y;
	}

	void Slider::SetPos( int x, int y, bool forceMove /*= false*/ )
	{
		Widget::SetPos(x,y,forceMove);
		CalculateSliderPos();
	}
}