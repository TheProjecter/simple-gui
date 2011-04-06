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

	protected:
		virtual void Update(float diff);

		Rect m_oldSizeMax;			//used to restore to previous max size
		bool m_maximized;
		bool m_minimized;
		virtual void SetPosFromDrag(Drag* drag);
		virtual void HandleDragStop(Drag* drag);
		virtual void UpdateClipArea();
	private:
	};
}