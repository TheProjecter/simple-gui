#include "../include/gui/TitleBar.hpp"
#include "../include/gui/Event.hpp"
#include "../include/gui/Button.hpp"
#include "../include/gui/GuiManager.hpp"

namespace gui
{
	class OnResize;

	TitleBar::TitleBar( Widget* parent , const std::string& text, 
						ButtonStyle buttonStyle)
	{
		//there can only be ONE titlebar per window, and this will be its name
		m_name = "my_titlebar";

		//title-bars can't be moved!
		m_movable = false;

		//don't save title-bars!
		m_allowSave = false;

		m_titleText = text;
		m_type = TITLE_BAR;

		//the title bar will be solid. other widgets will collide and won't 
		//be able to move the titlebar
		m_solid = true;

		//TODO: this is hardcoded value!
		m_titleVisibleText.SetSize(14);
		m_titleVisibleText.SetText(m_titleText);
		
		//check for size errors


		if(parent) {
			m_mediator.Connect(parent,"default",events::OnResize,false);
			m_mediator.Connect(parent,"default",events::OnMove,false);
		} else {
			error_log("CRASH ALERT: TitleBar initialized without parent!");
		}
		if(s_gui->GetTheme()) {
			m_rect = parent->GetRect();
			m_rect.h = 15; //TODO: this is a hardcoded value!

		} else {
			m_rect = parent->GetRect();
			m_rect.h = 15; //TODO: this is a hardcoded value!
		}

		//create buttons.. image buttons
 		if(buttonStyle & CLOSE) {
 			Button* button = NULL;
			button = new Button;
			button->AllowSave(false);
			button->SetName("close");
			button->SetText("X");
			button->Resize(30,20);
			this->AddWidget(button);
 		}

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
		if(s_gui->GetWindow()) s_gui->GetWindow()->Draw(m_titleVisibleText);
	}

	void TitleBar::InitGraphics()
	{
		if(s_gui->GetTheme()) {
			sf::Color upper = s_gui->GetTheme()->GetColor("titlebar-color1");
			sf::Color lower = s_gui->GetTheme()->GetColor("titlebar-color2");

			m_shape = TwoColoredRectangle(m_rect.w,m_rect.h,upper,lower);
			m_shape.SetPosition(GetPos());

			//maybe initiate the title string with something
			m_titleVisibleText.SetText("Testing Titlebar!");
		}
	}

	void TitleBar::Resize( int w, int h )
	{
		Widget::Resize(w,m_rect.h);	//don't resize the height!

		//TODO: this is hardcoded value.. FIXIT! 
		Widget* button = this->FindChildByName("close");
		if(button) {
			button->SetPos(m_rect.x+m_rect.w-button->GetRect().w,m_rect.y,true);
		}
	}

	void TitleBar::SetPos( int x, int y, bool forceMove /* = false */ )
	{
		Widget::SetPos(x,y,forceMove);	

		//TODO: this is hardcoded value.. FIXIT! 
		Widget* button = this->FindChildByName("close");
		if(button) {
			button->SetPos(m_rect.x+m_rect.w-button->GetRect().w,m_rect.y,forceMove);
		}		
	}

	void TitleBar::SetParent( Widget* parent )
	{
		if(!parent) return;

		Resize(parent->GetRect().w,parent->GetRect().h);
	}
}