#include "../include/gui/TitleBar.hpp"
#include "../include/gui/Event.hpp"
#include "../include/gui/Button.hpp"
#include "../include/gui/GuiManager.hpp"

namespace gui
{
	class OnResize;

	TitleBar::TitleBar( const std::string& text )
	{
		//there can only be ONE titlebar per window, and this will be its name
		m_name = "my_titlebar";
		m_type = TITLE_BAR;

		m_buttonStyle = DEFAULT;

		//title-bars can't be moved!
		m_movable = false;

		//TODO: hardcoded height? FIXIT!
		m_rect.h = 15;

		//don't save title-bars!
		m_allowSave = false;

		m_titleText = text;

		//the title bar will be solid. other widgets will collide and won't 
		//be able to move the titlebar
		m_solid = true;

		//TODO: this is hardcoded value!
		m_titleVisibleText.SetSize(14);
		m_titleVisibleText.SetText(m_titleText);
		

		InitGraphics();
	}

	void TitleBar::Update( float diff )
	{
		Widget::Update(diff);

		//parent events.. respond to them
		while(Event* e = m_mediator.GetEvent()) {
			switch(e->GetType()) {
				case events::OnResize:
					{
						gui::OnResize* r = static_cast<gui::OnResize*>(e);
						Widget* widget = r->GetWidget();
						bool needToResize = true;
						if(m_rect.w == widget->GetRect().w) 
							needToResize = false;

						if(needToResize)
							Resize(widget->GetRect().w,m_rect.h);

						//the window can't be smaller than the titlebar!
						if(widget->GetRect().h < m_rect.h) {
							widget->Resize(widget->GetRect().w,m_rect.h);
						}
					} break;
				case events::OnMove:
					{
						Widget* w = ((gui::OnResize*)e)->GetWidget();
						SetPos(w->GetRect().x, w->GetRect().y,true);
					} break;
				default: break;
			}
		}
	}

	void TitleBar::Draw() const
	{
		Widget::Draw();
		if(!s_gui) return;
			
		s_gui->GetWindow().Draw(m_titleVisibleText);
	}

	void TitleBar::InitGraphics()
	{
		if(!s_gui || !s_gui->GetTheme()) return;

		sf::Color upper = s_gui->GetTheme()->GetColor("titlebar-color1");
		sf::Color lower = s_gui->GetTheme()->GetColor("titlebar-color2");

		m_shape = TwoColoredRectangle(m_rect.w,m_rect.h,upper,lower);
		m_shape.SetPosition(GetPos());
		//TODO: this is hardcoded value!
		m_titleVisibleText.SetColor(sf::Color(194,133,241));
	}

	void TitleBar::Resize( int w, int h )
	{
		Widget::Resize(w,m_rect.h);	//don't resize the height!
		
		SetPos(m_rect.x,m_rect.y,true);	//tricky hack to move the buttons where they should be
	}

	void TitleBar::SetPos( int x, int y, bool forceMove /* = false */ )
	{
		Widget::SetPos(x,y,forceMove);	
		m_titleVisibleText.SetPosition((float)m_rect.x, (float)m_rect.y);

		//TODO: this is hardcoded value.. FIXIT! 
		if(m_buttonStyle & CLOSE) {
			Widget* button = NULL;
			button = this->FindChildByName("close");
			button->SetPos(m_rect.x+m_rect.w-button->GetRect().w,m_rect.y,true);
		}
		if(m_buttonStyle & MAXIMIZE) {
			Widget* button = NULL;
			sf::Vector2f tempPos;
			button = this->FindChildByName("close");
			if(button) {
				tempPos = button->GetRect().GetPos();
			}
			button = this->FindChildByName("max");
			button->SetPos((int)tempPos.x - button->GetRect().w, (int)m_rect.y,true);
		}
		if(m_buttonStyle & MINIMIZE) {
			Widget* button = NULL;
			sf::Vector2f tempPos;
			button = this->FindChildByName("max");
			if(button) {
				tempPos = button->GetRect().GetPos();
			} 
			button = this->FindChildByName("min");
			button->SetPos((int)tempPos.x - button->GetRect().w, (int)m_rect.y,true);
		} 
		if(m_buttonStyle & QUESTION) {
			Widget* button = NULL;
			sf::Vector2f tempPos;
			button = this->FindChildByName("min");
			if(button) {
				tempPos = button->GetRect().GetPos();
			} 
			button = this->FindChildByName("help");
			button->SetPos((int)tempPos.x - button->GetRect().w, (int)m_rect.y,true);
		}	
	}

	void TitleBar::SetParent( Widget* parent )
	{
		if(!parent) return;
		m_parent = parent;
		m_mediator.ClearConnections();

		m_mediator.Connect(parent,"default",events::OnResize,false);
		m_mediator.Connect(parent,"default",events::OnMove,false);

		Resize(parent->GetRect().w,15);	//hardcoded value
	}

	void TitleBar::SetTitleName( const std::string& title )
	{
		m_titleText = title;
		m_titleVisibleText.SetText(m_titleText);
	}

	void TitleBar::SetButtonStyle( ButtonStyle buttonStyle )
	{

		//save the button style
		m_buttonStyle = buttonStyle;

		//create buttons.. image buttons
		if(buttonStyle & CLOSE) {
			Button* button = NULL;
			button = new Button;
			button->AllowSave(false);
			button->SetName("close");
			button->SetText("X");
			button->Resize(15,15);
			this->AddWidget(button);
		}
		//create buttons.. image buttons
		if(buttonStyle & MINIMIZE) {
			Button* button = NULL;
			button = new Button;
			button->AllowSave(false);
			button->SetName("min");
			button->SetText("_");
			button->Resize(15,15);
			this->AddWidget(button);
		}
		//create buttons.. image buttons
		if(buttonStyle & MAXIMIZE) {
			Button* button = NULL;
			button = new Button;
			button->AllowSave(false);
			button->SetName("max");
			button->SetText("[]");
			button->Resize(15,15);
			this->AddWidget(button);
		}
		//create buttons.. image buttons
		if(buttonStyle & QUESTION) {
			Button* button = NULL;
			button = new Button;
			button->AllowSave(false);
			button->SetName("help");
			button->SetText("?");
			button->Resize(15,15);
			this->AddWidget(button);
		}
	}
}