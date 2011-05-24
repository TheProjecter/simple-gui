#include "../include/gui/Slider.hpp"
#include "../include/gui/GuiManager.hpp"

namespace gui 
{
	Slider::Slider() : m_curValue(0),m_lowerLimit(0), m_upperLimit(100), 
					   m_sliderDrag(false),m_hotSpotX(0),m_hotSpotY(0)
	{
		m_movable = false; 
		m_type = SLIDER;
		m_cursorRect.w = 15;
		m_cursorRect.h = 20;
		
		m_cursor = sf::Shape::Rectangle(0,0,(float)m_cursorRect.w, 
					(float)m_cursorRect.h,sf::Color(213,198,224));

		//buttons particular size hint
		m_sizeHint.x = 75;
		m_sizeHint.y = 25;

		//buttons particular size policy
		m_horizontalPolicy	= MinimumExpand;
		m_verticalPolicy	= Fixed;	

		SetLimits(m_lowerLimit,m_upperLimit);
	}

	Slider::Slider( int upper, int lower ) : m_curValue(0),m_lowerLimit(lower),
						m_upperLimit(upper), m_sliderDrag(false),m_hotSpotX(0),
						m_hotSpotY(0)
	{
		m_movable = false;
		m_type = SLIDER;
		m_cursorRect.w = 15;
		SetLimits(m_lowerLimit,m_upperLimit);
	}
	int Slider::GetValue()
	{
		return m_curValue;
	}

	void Slider::SetValue( int value )
	{

		if(value < m_lowerLimit) {
			value = m_lowerLimit;
		} else if(value > m_upperLimit) {
			value = m_upperLimit;
		}
		m_curValue = value;

		m_mediator.PostEvent(new OnValueChanged(this,value));

		CalculateSliderPos();
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
		CalculateSliderPos();
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
		Widget::OnClickPressed(event);

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
				SetValue(m_curValue + (m_upperLimit-m_lowerLimit)/10);
			} else {
				SetValue(m_curValue - (m_upperLimit-m_lowerLimit)/10);
			}
		}
	}

	void Slider::Update(float diff)
	{

		Widget::Update(diff);
	}

	void Slider::Draw() const
	{
		Widget::Draw();

		s_gui->GetWindow().Draw(m_cursor);
	}
	void Slider::OnClickReleased(sf::Event *event)
	{
		Widget::OnClickReleased(event);

		m_sliderDrag = false;
		CalculateSliderPos();
	}

	void Slider::OnOtherEvents(sf::Event* event)
	{
		Widget::OnOtherEvents(event);

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
				SetValue(m_upperLimit);
			} else if(landSpotX < m_rect.x) {
				landSpotX = m_rect.x;
				SetValue(m_lowerLimit);
			} else {
				double percent = (float)((landSpotX-m_rect.x)/(float)(m_rect.w-m_cursorRect.w));
				int maxNumbers = m_upperLimit - m_lowerLimit;

				SetValue(m_lowerLimit + int(maxNumbers*percent));
			}



			//now sets the position of the cursor
			m_cursorRect.x = landSpotX;

			CalculateSliderPos();
		}
	}

	void Slider::Resize(int w, int h, bool save /*=true*/)
	{
		Widget::Resize(w,h,save);
		m_cursorRect.h = h;
		m_cursor = sf::Shape::Rectangle(0,0,(float)m_cursorRect.w,(float)h,
					sf::Color(213,198,224));
	}

	void Slider::CalculateSliderPos()
	{
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

		m_cursor.SetPosition(sf::Vector2f((float)m_cursorRect.x,(float)m_cursorRect.y));
	}

	void Slider::SetPos( int x, int y, bool forceMove /*= false*/, bool save/*=true*/ )
	{
		Widget::SetPos(x,y,forceMove,save);
		CalculateSliderPos();
	}

	void Slider::ReloadSettings()
	{
		Widget::ReloadSettings();

		int32 upper = 100;
		int32 lower = 0;
		if(m_settings.HasInt32Value("upper-limit")) {
			upper = m_settings.GetInt32Value("upper-limit");
		}
		if(m_settings.HasInt32Value("lower-limit")) {
			lower = m_settings.GetInt32Value("lower-limit");
		}
		this->SetLimits(lower,upper);

		if(m_settings.HasInt32Value("value")) {
			int32 value = m_settings.GetInt32Value("value");
			this->SetValue(value);
		}
	}

	void Slider::InitGraphics()
	{
		if(m_settings.HasUint32Value("slider-background")) {
			m_shape.SetColor(UnsignedToColor(m_settings.GetUint32Value("slider-background")));
			m_individualTheme = true;
		} else if(s_gui->GetTheme()){
			m_shape.SetColor(s_gui->GetTheme()->GetColor("slider-background"));
 		}
// 		if(m_settings.HasUint32Value("slider-cursor")) {
// 			m_shape.SetColor(UnsignedToColor(m_settings.GetUint32Value("slider-cursor")));
// 			m_individualTheme = true;
// 		} else if(s_gui->GetTheme()){
// 			m_shape.SetColor(s_gui->GetTheme()->GetColor("slider-cursor"));
// 		}

	}
}