#include "../include/gui/Window.hpp"
#include "../include/gui/GuiManager.hpp"

namespace gui
{
	Window::Window( const std::string title, TitleBar::ButtonStyle buttons 
			/*= TitleBar::MIN_MAX_CLOSE*/ ): m_maximized(false),
			m_minimized(false)
	{
		m_type = WINDOW;

		TitleBar* titlebar = new TitleBar(title);
		if(!titlebar) {
			error_log("Unable to create a titlebar for some reason!");
			return;
		}

		if(buttons == TitleBar::DEFAULT) {
			//if the buttons style is default.. then it means that it'll 
			//get the button style from the saved settings of the window
		} else {
			titlebar->SetButtonStyle(buttons);
		}

		SetTitleName(title);
		AddWidget(titlebar);

		m_mediator.Connect(titlebar,"default",gui::events::OnDrag,false);
		m_mediator.Connect(titlebar,"default",events::OnDoubleClick,false);

	}

	void Window::Update( float diff )
	{
		Widget::Update(diff);

		while(Event* e = m_mediator.GetEvent()) {
			if(e->GetType() == sf::Event::MouseButtonPressed) {
				WidgetEvent* w = (WidgetEvent*)e;
				if(w->GetWidget()->GetName() == "close") {
					Kill();
					debug_log("Closing window \"%s\"",m_name.c_str());
				} else if(w->GetWidget()->GetName() == "min") {
					Minimize();
				} else if(w->GetWidget()->GetName() == "max") {
					Maximize();
				} else if(w->GetWidget()->GetName() == "help") {
					debug_log("Requiring help from window \"%s\"",m_name.c_str());
				}
			} else if(e->GetType() == gui::events::OnDrag) {
				Drag* drag = ((OnDrag*)e)->GetDrag();
				SetPos(drag->GetCurrentPos());
			} else if(e->GetType() == events::OnDoubleClick) {
				Maximize();
			}
		}
	}

	bool Window::SetPosFromDrag( Drag* drag )
	{
		//don't move from dragging.. only move when the titlebar was moved!
		return false;
	}

	bool Window::HandleDragStop( Drag* drag )
	{
		//basic widget drag-stop policy, other might do something different
		if(!drag) return false;
		if(drag->GetStatus() != Drag::Finished) return false;

		Widget* target = drag->GetTarget();

		//the titlebar movement is special.. don't count it
		if(target->GetType() == TITLE_BAR) return false;
		
		//if it's not the titlebar.. do the usual
		Widget::HandleDragStop(drag);
		
		return true;
	}

	void Window::UpdateClipArea()
	{
		if(!s_gui) return;

		if(m_minimized) {
			int temp = m_rect.h;
			//todo: hardcoded minimize value here:
			m_rect.h = 15;
			Widget::UpdateClipArea();
			m_rect.h = temp;
		} else {
			Widget::UpdateClipArea();
		}
	}

	void Window::Draw() const
	{
		if(!m_visible) return;

		StartClipping();
			s_gui->GetWindow().Draw(m_shape);
		StopClipping();
		
		//also draw children if any
		for(WidgetList::const_iterator it = m_widgets.begin(); it != m_widgets.end(); it++) {
			//don't draw outside parent's rect
			StartClipping();
				it->second->Draw();
			StopClipping();
		}

	}

	void Window::ReloadSettings()
	{
		Widget::ReloadSettings();

		if(m_settings.HasStringValue("title")) {
			SetTitleName(m_settings.GetStringValue("title"));
		}
		if(m_settings.HasUint32Value("button-style")) {
			SetButtonStyle(TitleBar::ButtonStyle(m_settings.GetUint32Value("button-style")));
		}

	}

	void Window::SetTitleName( const std::string& title_name )
	{
		TitleBar* titlebar = dynamic_cast<TitleBar*>(this->FindChildByName("my_titlebar"));

		if(!titlebar) 
			return;

		m_settings.SetStringValue("title",title_name);
		titlebar->SetTitleName(title_name);
	}

	void Window::SetButtonStyle( TitleBar::ButtonStyle buttons )
	{
		TitleBar* titlebar = dynamic_cast<TitleBar*>(this->FindChildByName("my_titlebar"));

		if(!titlebar) 
			return;

		titlebar->SetButtonStyle(buttons);
		m_settings.SetUint32Value("button-style",buttons);

		if(buttons & TitleBar::CLOSE)
			m_mediator.Connect(titlebar,"close","default",sf::Event::MouseButtonPressed,false);
		if(buttons & TitleBar::MINIMIZE)
			m_mediator.Connect(titlebar,"min",  "default",sf::Event::MouseButtonPressed,false);
		if(buttons & TitleBar::MAXIMIZE)
			m_mediator.Connect(titlebar,"max",  "default",sf::Event::MouseButtonPressed,false);
		if(buttons & TitleBar::QUESTION)
			m_mediator.Connect(titlebar,"help", "default",sf::Event::MouseButtonPressed,false);

	}

	void Window::Minimize()
	{
		m_minimized = ! m_minimized;

		UpdateClipArea();
		debug_log("Minimizing window \"%s\"",m_name.c_str());
	}

	void Window::Maximize()
	{
		if(!m_maximized) {
			m_oldSizeMax = m_rect;
			SetPos(0,0,true);
			Resize(s_gui->GetWindow().GetWidth(),s_gui->GetWindow().GetHeight());
			m_maximized = true;
		} else { //restored
			m_rect = m_oldSizeMax;
			SetPos(m_rect.x,m_rect.y,true);
			Resize(m_rect.w,m_rect.h);
			m_maximized = false;
		}
		debug_log("Maximizing window \"%s\"", m_name.c_str());
	}
}