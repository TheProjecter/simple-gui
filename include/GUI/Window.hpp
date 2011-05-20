#pragma once

#include "TitleBar.hpp"

namespace gui
{
	class Widget;
	class Window : public Widget
	{
	public:
		Window(const std::string title, TitleBar::ButtonStyle buttons = TitleBar::MIN_MAX_CLOSE);

		virtual void Draw() const;

		void SetTitleName(const std::string& title_name);

	protected:
		virtual void Update(float diff);

		Rect m_oldSizeMax;			//used to restore to previous max size
		bool m_maximized;
		bool m_minimized;
		virtual bool SetPosFromDrag(Drag* drag);
		virtual bool HandleDragStop(Drag* drag);
		virtual void UpdateClipArea();
		virtual void ReloadSettings();
		void SetButtonStyle(TitleBar::ButtonStyle button_style);
	private:
	};
}