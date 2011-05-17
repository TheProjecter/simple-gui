#include "../include/gui/CheckBox.hpp"

namespace gui
{	
	CheckBox::CheckBox() : m_checked(false),m_uncheckedImage(NULL)
	{
		m_type = CHECKBOX;



		//buttons particular size hint
		m_sizeHint.x = 75;
		m_sizeHint.y = 25;

		//buttons particular size policy
		m_horizontalPolicy	= MinimumExpand;
		m_verticalPolicy	= Fixed;	
	}

	bool CheckBox::IsChecked() const
	{
		return m_checked;
	}

	void CheckBox::Check()
	{
		m_checked = true;

	}

	void CheckBox::UnCheck()
	{
		m_checked = false;
	}

	void CheckBox::Draw()
	{
// 		if(m_checked) 
// 			Widget::Draw();
// 		else Widget::Draw(m_uncheckedImage);
	}

/*
	void CheckBox::SetDefaultImage()
	{
		sprite.SetImage(ImageManager::GetSingleton().GetImage("assets/gfx/checkbox1.png","default_checked"));
		uncheckedSprite.SetImage(ImageManager::GetSingleton().GetImage("assets/gfx/checkbox2.png","default_unchecked"));

		pos.w = sprite.GetSize().x; pos.h = sprite.GetSize().y;	
	}
*/

}