#pragma once

#include "Widget.hpp"
#include <vector>

namespace gui 
{
	struct Word {
		float GetWidth() const;
		void SetPos(int x, int y);
		std::vector<sf::String> m_char;
	};

	struct Line {
		Line(): m_align(ALIGN_LEFT) {}
		enum LineAlignment {
			ALIGN_LEFT,
			ALIGN_RIGHT,
			ALIGN_CENTER
		};
		float GetWidth() const;
		void SetPos(const Rect& container,uint32 line_spacing);
		uint32 GetLineSpacing() const;
		std::vector<Word> m_words;
		LineAlignment m_align;	
	};

	class TextArea : public Widget
	{
	public:
		TextArea();
		void SetText(const std::string& text);
		void Resize(int w, int h, bool save = true);
		void SetPos(int x, int y, bool forceMove  = false ,bool save=true);
	private:
		std::string m_text;
		std::vector<Line> m_lines;
		uint32 m_viewableLines;
		uint32 m_totalLines;
		uint32 m_startingLine;
		
		virtual void Draw() const;
		virtual void InitGraphics();
	};

	void Parse(std::stringstream& s,std::vector<Line>& text, const Rect& container);
}