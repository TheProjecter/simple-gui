#pragma once 

#include <sfml/System.hpp>
#include <sfml/Graphics.hpp>

namespace gui {

	/* Useful primitive data typedefs */
	typedef sf::Uint32 uint32;
	typedef sf::Uint16 uint16;
	typedef sf::Uint8   uint8;

	typedef sf::Int32   int32;
	typedef sf::Int16   int16;
	typedef sf::Int8	 int8;

	enum WidgetType {
		WIDGET, 
		BUTTON, 
		IMAGE_BUTTON,
		LABEL, 
		LINE_EDIT, 
		CHECKBOX, 
		RADIOBOX, 
		SLIDER,
		TITLE_BAR,
		WIDGETS_COUNT
	};

	namespace events {
		enum Events {

			/* Widget specific events */
			OnResize = sf::Event::Count,
			OnMove,
			OnShow,
			OnHide,
			OnDestroy,
			OnHover,
			OnHoverLost,
			OnFocus,
			OnFocusLost,

			Count
		};
	}
	enum Alignment {
		None			 = 0,
		Left			 = 0x0001,
		Right			 = 0x0010,
		Top				 = 0x0100,
		Bottom			 = 0x1000,
		CenterHorizontal = 0x0011,  //Left | Right
		CenterVertical	 = 0x1100,	//Top  | Down
		Center			 = 0x1111,	//Top  | Down | Left | Right
		LeftBottom		 = 0x1001,	//Left | Bottom
		RightBottom		 = 0x1010,
		LeftTop			 = 0x0101,
		RightTop		 = 0x0110
	};
	
	/* A useful callback structure -- 
	 * inherit it if you want to use callbacks
	 */
	struct Functor {
		virtual void operator()();
	};



	/* A simple and useful Rect class */
	struct Rect {
		Rect(int X=0, int Y=0,int W=0, int H=0);
		Rect(const sf::Vector2i& pos, const sf::Vector2i& size);
		Rect(const sf::Vector2f& pos, const sf::Vector2f& size);
		Rect(const Rect& rect);
		Rect(const sf::IntRect& rect);
		
		//overloaded operators
		const Rect operator+(const Rect& other) const;
		const Rect operator+(int panning) const;
		const Rect operator-(const Rect& other) const;
		const Rect operator-(int panning) const;
		Rect& operator+=(const Rect& other);
		Rect& operator+=(int panning);
		Rect& operator-=(const Rect& other);
		Rect& operator-=(int panning);
		bool operator==(const Rect& other) const;
		bool operator!=(const Rect& other) const;
		Rect& operator=(const Rect& other);
		bool operator!() const;

		sf::Vector2f GetPos() const;
		sf::Vector2i GetSize() const;

		//conversion operator with sf::IntRect
		operator sf::IntRect() const;
		int x,y,w,h;
	};

	sf::Shape TwoColoredRectangle(int width, int height, sf::Color upper, sf::Color lower);

	float ComputeDistance(const sf::Vector2f& first, const sf::Vector2f& second);

	/* A classic rect-collision function */
	bool IsCollision(const Rect& first, const Rect& second);
	bool IsCollision(const Rect& first, const Rect& second, sf::Vector2f& normal);

	std::string ToUpper(const std::string& text);
	sf::Color UnsignedToColor(uint32 rgba);
	uint32 ColorToUnsigned(const sf::Color& color);

	void InsertChar(std::ostream& os, char c, uint32 count);
}

std::ostream& operator<<( std::ostream& os, gui::Rect& rect);
std::stringstream& operator << (std::stringstream& ss, gui::Rect& rect);