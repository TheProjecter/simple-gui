#pragma once

#include "Widget.hpp"
#include <vector>

namespace gui 
{
	struct Word {
		float GetWidth() const;
		std::string GetText() const;
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
		std::string GetText() const;

		void SetPos(const Rect& container,uint32 line_spacing);
		uint32 GetLineSpacing() const;

		std::vector<Word> m_words;
		LineAlignment m_align;	
	};

//***********************************************************************
//*						Text Area Specific Tags							*
//***********************************************************************
// 1. <color>	must be closed by a </color>						    *
//	  usage:															*
//		<color=x.x.x> The rgb components will be r=x, g=x, b=x			*
//		The default value for x is 0 in case it's not specified.        *
//		Example: <color=255> r=255,g=0,b=0 => red!					    *
// 2. <font size> must be closed by a </font size>						*
//	  usage:															*
//		<font size=x> The size of the text will be x until you close	*
//		the tag															*
// 3. <align=x> must be closed by a </align>							*
//	  usage:															*
//		<align=left>	Aligns text to the left.						*
//		<align=right>   Aligns text to the right.						*
//		<align=center>	Aligns text to the center.						*
//	  Warning:															*
//		 The current line on which you specified the alignement will	*
//		keep the alignment even if you close the tag on the same line!	*
// 4. <b> must be closed by a </b>										*
//	  Makes the text have a bold style until closed						*
//	  usage:															*
//		<b>Some Bold Text</b>											*
// 5. <i> must be closed by a </i>										*
//	  Makes the text have an italic style until closed					*
//	  usage:															*
//		<i>Some Italic Text</i>											*
// 6. <u> must be closed by a </u>										*
//	  Makes the text have a underlined style until closed				*
//	  usage:															*
//		<u>Some Underlined Text</u>										*
//																		*
//***********************************************************************
	
	class TextArea : public Widget
	{
	public:
		TextArea();
		void SetText(const std::string& text);
		std::string GetTextFromLine(uint32 line) const;
		const std::string& GetText() const;
		void Resize(int w, int h, bool save = true);
		void SetPos(int x, int y, bool forceMove = false, bool save=true);
	private:
		std::string m_text;
		std::vector<Line> m_lines;
		uint32 m_viewableLines;
		uint32 m_totalLines;
		uint32 m_startingLine;
		
		virtual void Draw() const;
		virtual void InitGraphics();
		virtual void ReloadSettings();
	};

	void Parse(std::stringstream& s,std::vector<Line>& text, const Rect& container);
}