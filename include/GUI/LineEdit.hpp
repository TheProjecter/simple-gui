#pragma once

#include "Widget.hpp"

namespace gui {

	class LineEdit: public Widget 
	{
	public:
		LineEdit();
		void SetBackgroundColor(sf::Color color);
		void SetPos(int x, int y,bool forceMove = false);
		void SetPasswordField(bool flag);
	private:
		void Draw() const;
		void _SetCursorPos();
		uint32 _FindCursorPos(const std::string& text, uint32 pos,int searchType);
	protected:
		void Update(float diff);
		void SetVisibleText();
		void TestSizeErrors(bool moveStartIndex = false);
		void OnKeyPressed(sf::Event* event);
		void OnTextEntered(sf::Event* event);

		sf::String m_visibleText;
		std::string m_totalText;
		uint32 m_allowedChars;
		bool m_isPassword;

		/* Cursor's attributes */
		sf::Shape m_cursor;
		mutable float m_cursorDiff;		//cursor blink update diff
		uint32 m_cursorIndex;	//the position on the total text
		uint32 m_visibleChars;	//the amount characters that can be displayed
		uint32 m_cursorStartIndex; 
		mutable bool m_cursorShow;
	};

}