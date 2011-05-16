#pragma once

#include "Widget.hpp"

namespace gui 
{
	class TitleBar : public Widget
	{
	public:
		enum ButtonStyle {
			CLOSE	 = 1 << 0,
			MAXIMIZE = 1 << 1,
			MINIMIZE = 1 << 2,
			QUESTION = 1 << 3,
			MIN_MAX_CLOSE = MINIMIZE | MAXIMIZE | CLOSE,
			ALL = 0xFF
		};

		TitleBar(const std::string& = "TitleBar", ButtonStyle buttonStyle = MIN_MAX_CLOSE);

		void Resize(int w, int h);
		void SetPos(int x, int y, bool forceMove /* = false */);

		void SetParent(Widget* parent);
	protected:
		virtual void Update(float diff);
		virtual void Draw() const;
		void InitGraphics();
		virtual bool HandleDragMove(Drag* drag) { return false; }	//don't handle drags
		virtual bool HandleDragStop(Drag* drag) { return false; }

	private:
		ButtonStyle m_buttonStyle;
		std::string m_titleText;
		sf::String m_titleVisibleText;
	};

}