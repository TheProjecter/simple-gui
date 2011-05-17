#include "../include/gui/RadioBox.hpp"
#include <iostream>


namespace gui
{	
	void RadioBox::Draw() const
	{

		//if(!m_checked) 
			Widget::Draw();
		//else Widget::Draw(m_uncheckedImage);		
	}

/*
	void RadioBox::SetDefaultImage()
	{
		sprite.SetImage(ImageManager::GetSingleton().GetImage("assets/gfx/checkbox1.png","default_checked"));
		uncheckedSprite.SetImage(ImageManager::GetSingleton().GetImage("assets/gfx/radiobox2.png","default_unchecked_radio"));

		pos.w = (int)sprite.GetSize().x; pos.h = (int)sprite.GetSize().y;	
	}
*/

	void RadioBox::Check()
	{
		m_checked = true;

		//if you don't have a parent, no reason to uncheck other radioboxes
		if(!m_parent) return;

		WidgetList& Ui = m_parent->GetWidgetList();
		for(WidgetList::const_iterator i = Ui.begin(); i != Ui.end(); i++) {
			if(i->second->GetType() == RADIOBOX && i->second != this) {
				((RadioBox*)i->second)->UnCheck();
			}
		}		
	}	

	RadioBox::RadioBox() : m_checked(false)
	{
		m_type = RADIOBOX;

		//buttons particular size hint
		m_sizeHint.x = 75;
		m_sizeHint.y = 25;

		//buttons particular size policy
		m_horizontalPolicy	= MinimumExpand;
		m_verticalPolicy	= Fixed;	
	}

	bool RadioBox::IsChecked()
	{
		return m_checked;
	}

	void RadioBox::UnCheck()
	{
		m_checked = false;
	}

	void RadioBox::OnClickPressed( sf::Event* event )
	{
		Widget::OnClickPressed(event);
		Check();
	}
}