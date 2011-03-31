#pragma once

#include "TitleBar.hpp"

namespace gui
{
	class Widget;
	class Window : public Widget
	{
	public:
		Window(const std::string title, TitleBar::ButtonStyle buttons = TitleBar::MIN_MAX_CLOSE);

		virtual void Update(float diff);

	protected:
		Rect m_oldSizeMax;			//used to restore to previous max size
		Rect m_oldSizeMin;			//used to restored to previous min size
		bool m_maximized;
		bool m_minimized;
		virtual void SetPosFromDrag(Drag* drag);
		virtual void HandleDragStop(Drag* drag);
	private:
	};
}