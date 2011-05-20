#include "../include/gui/TextArea.hpp"
#include "../include/gui/GuiManager.hpp"
#include <sstream>
#include <iostream>
#include <vector>
#include <stack>


void gui::TextArea::SetText( const std::string& text )
{
	m_text = text;
	m_type = TEXT_AREA;
	std::stringstream s;
	s.str(text);
	Parse(s,m_lines,m_rect);
	m_totalLines = m_lines.size();
	m_viewableLines = m_totalLines;
	SetPos(m_rect.x,m_rect.y,true);
}

void gui::TextArea::Draw() const
{
	Widget::Draw();

	for(uint32 i=m_startingLine; i<m_lines.size(); i++) {
		const Line& line = m_lines[i];
		for(uint32 j=0; j<line.m_words.size(); j++) {
			const Word& word = line.m_words[j];
			for(uint32 k=0; k<word.m_char.size(); k++) {
				const sf::String& string = word.m_char[k];
				s_gui->GetWindow().Draw(string);
			}
		}
	}
}

gui::TextArea::TextArea(): m_viewableLines(1),m_totalLines(1),
						   m_startingLine(0)
{

	//buttons particular size hint
	m_sizeHint.x = 320;
	m_sizeHint.y = 240;

	m_type = TEXT_AREA;

	//buttons particular size policy
	m_horizontalPolicy	= MaximumExpand;
	m_verticalPolicy	= MaximumExpand;	
}

void gui::TextArea::Resize( int w, int h , bool save /*=true*/)
{
	Widget::Resize(w,h,save);
	std::stringstream s; 
	s.str(m_text);
	Parse(s,m_lines,m_rect);	//this will also set the pos of the text
	SetPos(m_rect.x,m_rect.y,true);
}

void gui::TextArea::SetPos( int x, int y, bool forceMove, /* = false */
							bool save /*=true*/)
{
	Widget::SetPos(x,y,forceMove,save);
	uint32 line_spacing = 0;
	for(uint32 i=0; i<m_lines.size(); i++) {
		//find the spacing for the next line
		if(i>0) {
			line_spacing += m_lines[i-1].GetLineSpacing();
		}
		m_lines[i].SetPos(m_rect,line_spacing);
	}
}

void gui::TextArea::InitGraphics()
{	
	//DEBUG!!: hardcoded value!
	m_shape.SetColor(sf::Color(255,255,255));

}

inline float char_width(char* c, const sf::Font& font)
{
	sf::String temp;
	temp.SetFont(font);
	temp.SetText(c);
	return temp.GetRect().GetWidth();
}
inline float char_height(char* c, const sf::Font& font)
{
	sf::String temp;
	temp.SetFont(font);
	temp.SetText(c);
	return temp.GetRect().GetHeight();
}
void gui::Parse( std::stringstream& s,	//the text to be parsed
				std::vector<Line>& text,		//here will be added the lines
				const Rect& container )			//the space-restriction rect
{

	//delete all previous words
	text.clear();

	sf::String word;
	Line myLine;
	Word myWord;

	uint32 styles = 0;
	std::stack<sf::Color> colors;
	std::stack<uint32> fontSizes;

	std::stack<Line::LineAlignment> aligns;
	aligns.push(Line::ALIGN_LEFT);
	colors.push(sf::Color(0,0,0));
	fontSizes.push(14);

	bool changedStyle = true;
	bool changedAlignment = true;
	bool changedSize = true;


	word.SetText(" ");

	word.SetText("");
	bool newline = false;
	std::string line;
	while(std::getline(s,line)) {
		line+='\n';	//add the lost \n character

		//iterate through the line and parse it
		for(uint32 i=0; i<line.size(); i++) {
			char c = line[i];
			//if it is a tag then parse the tag and update style flags
			if(c == '<') {
				while(i<line.size()) {

					//skip white-spaces
					while(i<line.size() && line[i] == ' ') ++i;

					///////////////////////////////////////////////////////
					////			 the opening '<' tags    		   ////
					///////////////////////////////////////////////////////
					//if it's the <b > flag.. setup flags, and find the closing '>'
					if(i+1 < line.size() && line[i+1] == 'b') {
						i++;
						//setup flags
						styles |= sf::String::Bold;
						
						changedStyle = true;
						bool found = false;
						//find matching '>'
						while(!found) {
							if(i<line.size() && line[i] == '>')  {
								found = true;
								break;
							} else if(i >= line.size()) {
								std::getline(s,line);
								line+='\n';	//add the lost \n character
								i=0;
							}
							++i;
						}
						if(found) break; //break out of the while(i<line.size())
					} else if(i+1<line.size() && line[i+1] == 'i') {
						i++;
						//setup flags
						styles |= sf::String::Italic;

						changedStyle = true;

						bool found = false;
						//find matching '>'
						while(!found) {
							if(i<line.size() && line[i] == '>')  {
								found = true; break;
							} else if(i >= line.size()) {
								std::getline(s,line);
								line+='\n';	//add the lost \n character
								i=0;
							}
							++i;
						}
						if(found) break; //break out of the while(i<line.size())
						//if you didn't find it.. get a newline until you find the matching '>'
					} else if(i+1<line.size() && line[i+1] == 'u') {
						i++;
						//setup flags
						styles |= sf::String::Underlined;

						changedStyle = true;

						bool found = false;
						//find matching '>'
						while(!found) {
							if(i<line.size() && line[i] == '>')  {
								break;
								found = true;
							} else if(i >= line.size()) {
								std::getline(s,line);
								line+='\n';	//add the lost \n character
								i=0;
							}
							++i;
						}
						if(found) break; //break out of the while(i<line.size())
					} else if(i+5<line.size()) {
						//<font>
						++i;
						std::string temp;
						temp = line.substr(i,4);
						temp = ToUpper(temp);
						if(temp == "FONT") {
							bool found = false;
							bool selectingSize = false;
							uint32 size = 14;
							std::string sSize;
							temp.clear(); i+=4;
							//find the '=' op.. or break on closing '>' tag
							while(!found) {
								if(i >= line.size()) {
									std::getline(s,line);
									i = 0;
									line += '\n';
									if(!line.size() && s.eof()) {
										found = true; break;
									}
								}
								if(selectingSize && isdigit(line[i])) {
									sSize += line[i];
								} else if(selectingSize && !isdigit(line[i]) && sSize.size()) {
									//finished parsing the size
									selectingSize = false;
									size = (uint32)atoi(sSize.c_str());
									fontSizes.push(size);
									changedSize = true;
								}
								if(line[i] == '=') {
									temp = ToUpper(temp);
									//<font size = 24>
									if(temp.find("SIZE") != std::string::npos) 
										selectingSize = true;
								} else if(line[i] == '>') {
									found = true;
									break;
								} else temp += line[i];

								++i;
							}
							if(found) break;
						}
						temp = line.substr(i,5);
						temp = ToUpper(temp);
						//<color>
						if(temp == "COLOR") 
						{

							bool found = false;
							changedStyle = true;

							//find the '=' operator
							while(i<line.size() && line[i] != '=') ++i;
							++i;

							int colorsCount = 0;
							int colorDigitCount = 0;
							sf::Color color;

							std::stringstream ss;
							//loop until you find the '>' end tag
							while(!found) {

								//if you already picked 3 colors+alpha.. then skip everything until you find the '>'
								if(colorsCount >= 4) { ++i; continue; }

								//skip white-space, tabs,and punctuation
								if(i<line.size() && (line[i] == ' '  || 
									  ispunct(line[i]) || line[i] == '\t' ||
									  line[i] == '>')) 
								{
									//if a digit was found before.. then a separator, then it's a new color!
									if(colorDigitCount > 0) {
										//set the color
										uint32 tempcolor = 0;
										ss >> tempcolor;
										ss.str("");
										ss.clear();
										ss.flush();
										if(tempcolor > 255) tempcolor = 255;

										if(colorsCount == 0)	  color.r = (uint8)tempcolor;
										else if(colorsCount == 1) color.g = (uint8)tempcolor;
										else if(colorsCount == 2) color.b = (uint8)tempcolor;
										else if(colorsCount == 3) color.a = (uint8)tempcolor;

										++colorsCount;	//another color was set

										colorDigitCount = 0;
									}
									if(i<line.size() && line[i]=='>') {
										found = true;
										break;
									}
									++i;	//skip delimiters
								}

								//if you finished the line.. and still haven't found the '>'.. get a new line
								if(i >= line.size()) {
									std::getline(s,line);
									line+='\n';	//add the lost \n character
									i = 0;
									continue;
								}

								if(isdigit(line[i])) {
									//if at least 3 digits were already 
									// found.. create a new color then
									// skip the rest of the digits until ' ' or '.' or ','
									if(colorDigitCount >= 3) { 
										colorDigitCount = 0;
										//set the color
										uint16 tempcolor;
										ss >> tempcolor;
										if(tempcolor > 255) tempcolor = 255;

										if(colorsCount == 0)	  color.r = (uint8)tempcolor;
										else if(colorsCount == 1) color.g = (uint8)tempcolor;
										else if(colorsCount == 2) color.b = (uint8)tempcolor;
										else if(colorsCount == 3) color.a = (uint8)tempcolor;
										
										++colorsCount;	//another color was set

										++i;		//we won't consider any other character until we find
										continue;	//a delimiter
									} else {	//not 3 digits were found..
										//get the number
										char c = line[i];
										ss << c;
										++colorDigitCount;
									}
								} //end of if(isdigit());							

								++i;
							} //end of searching for '>' while(!'>').. meaning we found it .. or eof!

							//by now.. the color should be set!
							colors.push(color);
							
							//setup flags
							changedStyle = true;
							break; 
							
						} //end of if("color");
						else if(temp == "ALIGN") {
							changedAlignment = true;

							i+=5;	//jump over the align word
							bool chosen = false;	//if the align style was already chosen

							//loop until the matching '>' tag
							while(i<line.size() && line[i] != '>') 
							{
								if(line[i] == '>') break;


								//if the alignment style was already chosen, 
								// then skip all character until '>' or eof
								if(chosen) {
									++i;
									if(i>=line.size()) {
										std::getline(s,line);
										line+='\n';	//add the lost \n character
										i=0;
									}
									continue;
								}

								
								if(i+6<line.size() && (ToUpper(line.substr(i,6)) == "CENTER")) {
									aligns.push(Line::ALIGN_CENTER);
									chosen = true;
								} else if(i+4<line.size() &&(ToUpper(line.substr(i,4)) == "LEFT")) {
									aligns.push(Line::ALIGN_LEFT);
									chosen = true;
								} else if(i+5<line.size() &&(ToUpper(line.substr(i,5)) == "RIGHT")) {
									aligns.push(Line::ALIGN_RIGHT);
									chosen = true;
								} 
								
								++i;								
								//if you finished the current line..
								//and still haven't found the '>' get a newline
								if(i>=line.size()) {
									std::getline(s,line);
									line+='\n';	//add the lost \n character
									i=0;
								}
							}
							if(chosen) break;
						} //end of if("align")
					}	//end of if(size>5);

					///////////////////////////////////////////////////////
					//// 		   the closing '</' tags		       ////
					///////////////////////////////////////////////////////
					bool findMatchingTag = false;
					bool found = false;

					if(i<line.size() && line[i] == '/') {

						//loop until you find the matching '>' tag
						while(!found) {

							//the tag was closed
							if(i<line.size() && line[i] == '>'){
								found = true;
								break;
							}

							//if you got to the end of the line get a new one
							if(i>=line.size()) {
								std::getline(s,line);
								line+='\n';	//add the lost \n character
								i=0;
								continue;
							}

							//skip everything else if you're searching for
							//the matching '>' tag
							if(findMatchingTag) { ++i; continue; }



							//the closing </ b> tag
							if(i<line.size() && line[i] == 'b') {
								findMatchingTag = true;
								changedStyle = true;
								styles &= ~sf::String::Bold;
							} else if(i<line.size() && line[i] == 'i') {
								findMatchingTag = true;
								changedStyle = true;
								styles &= ~sf::String::Italic;
							} else if(i<line.size() && line[i] == 'u') {
								findMatchingTag = true;
								changedStyle = true;
								styles &= ~sf::String::Underlined;
							} else if(i+5<line.size() && 
								ToUpper(line.substr(i,5)) == "ALIGN") {
								findMatchingTag = true;
								changedAlignment = true;
								if(aligns.size()) {
									aligns.pop();
								}
							} else if(i+5<line.size() && 
								ToUpper(line.substr(i,5)) == "COLOR") {
								findMatchingTag = true;
								changedStyle = true;
								if(colors.size()) {
									colors.pop();
								}
							} else if(i+4<line.size() &&
								ToUpper(line.substr(i,4)) == "FONT") {
									i+=5;	//skip the white-space too
									if(i+4<line.size() &&
										ToUpper(line.substr(i,4)) == "SIZE") {
											changedSize = true;
											if(fontSizes.size()) {
												fontSizes.pop();
											}
									}
							}
							++i;
						} //end of searching for matching '>' while loop
						if(found) break;
					} //end of if('/') closing tags

					++i;
				} //end of while(i<line.size());
			} else {
				std::string temp = word.GetText();
				switch(c) {
					case ' ':	//finish the word and add it to the line
					{
						temp+=c;
						word.SetText(temp);

						myWord.m_char.push_back(word);
						word.SetText("");
						myLine.m_words.push_back(myWord);
						myWord.m_char.clear();
					} break;
					case '\t':
					{
						if((int)(myLine.GetWidth() + char_width("\t",word.GetFont())) > container.w) {
							myWord.m_char.push_back(word);
							word.SetText("");
							myLine.m_words.push_back(myWord);
							myWord.m_char.clear();
							text.push_back(myLine);
							myLine.m_words.clear();
						} else {
							std::string t = word.GetText();
							t+=c;
							word.SetText(t);

							myWord.m_char.push_back(word);
							myLine.m_words.push_back(myWord);
							myWord.m_char.clear();

						}
					} break;
					case '\n':	
					{
						myWord.m_char.push_back(word);
						word.SetText("");
						myLine.m_words.push_back(myWord);
						myWord.m_char.clear();
						text.push_back(myLine);
						myLine.m_words.clear();
					} break;
					default:	//it's a regular character.. add it to the word
					{
						if(changedStyle) {

							//don't push a word unless it contains something
							if(temp.size()) {
								myWord.m_char.push_back(word);
							}
							std::string t;
							t += c;
							word.SetText(t);

							if(colors.size()) {
								sf::Color c = colors.top();
								word.SetColor(c);
							}
							word.SetStyle(styles);
							changedStyle = false;
						} else {
							temp += c;
							word.SetText(temp);
						}
						if(changedAlignment && aligns.size()) {
							myLine.m_align = aligns.top();
							changedAlignment = false;
						}
						if(changedSize && fontSizes.size()) {
							//don't push a word unless it contains something
							if(temp.size()) {
								myWord.m_char.push_back(word);
							}
							std::string t;
							t += c;
							word.SetText(t);

							word.SetSize((float)fontSizes.top());
							changedSize = false;
						}
						//if the current line doesn't have enough space to 
						//hold the current word, then create a new line
						if((int)(myLine.GetWidth() + word.GetRect().GetWidth()) > container.w) {
							text.push_back(myLine);
							myLine.m_words.clear();
						} 
					} break;
				}
			}
		}
	}
	//push the last word on the last line if there's a word on it .. ?
	if(word.GetSize()) {
		myWord.m_char.push_back(word);
		if(text.size()) {
			text[text.size()-1].m_words.push_back(myWord);
		} else {
			myLine.m_words.push_back(myWord);
			text.push_back(myLine);
		}
	}

	//set the positions of the words
	for(uint32 i=0; i<text.size(); i++) {
		text[i].SetPos(container,i);
	}
}

float gui::Word::GetWidth() const
{
	float temp = 0;
	for(uint16 i=0; i<m_char.size();i++) {
		temp += m_char[i].GetRect().GetWidth();
	}
	return temp;
}

void gui::Word::SetPos( int x, int y )
{
	float width = 0;
	for(uint32 i=0; i<m_char.size();i++) {
		m_char[i].SetPosition((float)x+width,(float)y);
		width += m_char[i].GetRect().GetWidth();
	}
}
float gui::Line::GetWidth() const
{
	float temp = 0;
	for(uint32 i =0; i<m_words.size(); i++) {
		temp += m_words[i].GetWidth();
	}
	return temp;
}

void gui::Line::SetPos( const Rect& container,uint32 line_spacing)
{
	sf::Vector2f pos;
	switch (m_align)
	{
		case ALIGN_LEFT:
		{
			pos.x = (float)container.x;
		} break;
		case ALIGN_RIGHT:
		{
			pos.x = (float)container.x+(float)container.w;
			pos.x -=(float)GetWidth();
		} break;
		case ALIGN_CENTER:
		{
			pos.x = (float)container.x+(float)container.w/2;
			pos.x -=(float)GetWidth()/2;
		} break;
		default: break;
	}		
	float widthSum = 0;
	sf::Vector2f temp;
	temp.y = (float)container.y + (float)line_spacing;
	for(uint32 i=0; i<m_words.size();i++) {
		temp.x = pos.x;

		temp.x += (float)widthSum;
		widthSum += m_words[i].GetWidth();
		m_words[i].SetPos((int)temp.x,(int)temp.y);
	}

}

gui::uint32 gui::Line::GetLineSpacing() const
{
	uint32 max = 0;
	sf::String temp;
	temp.SetText("\n");
	for(uint32 i=0; i<m_words.size(); i++) {
		uint32 candidate = 0;
		const Word& word = m_words[i];
		for(uint32 j=0; j< word.m_char.size(); j++) {
			temp.SetSize(word.m_char[j].GetSize());
			temp.SetStyle(word.m_char[j].GetStyle());
			candidate = (uint32)temp.GetRect().GetHeight();
			max = std::max(max,candidate);
		}
	}
	return max;
}