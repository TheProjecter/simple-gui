#include "../include/gui/Defines.hpp"
#include <sstream>
#include <iostream>
#include <cmath>

namespace gui {

	bool IsCollision( const Rect& first, const Rect& second )
	{
		return (    first.x < second.x + second.w &&
			first.x + first.w > second.x  &&
			first.y < second.y + second.h &&
			first.y + first.h > second.y         
			);	
	}

	bool IsCollision( const Rect& a1, const Rect& b1, sf::Vector2f& normal )
	{
// 	   Since the vector is a pointer then it could have been
// 	   initialized to any value so we need to make sure we initialize
// 	   the vectors X and Y to zero
		   
		Rect a(a1), b(b1);
		//move to center
		a.x = a.x + a.w/2;
		a.y = a.y + a.h/2;

		b.x = b.x + b.w/2;
		b.y = b.y + b.h/2;

	   normal.x = 0;
	   normal.y = 0;

	   //The distance between the two objects
	   sf::Vector2f Distance;
	   //The absDistance between the objects
	   sf::Vector2f absDistance;

	   float XMagnitute;
	   float YMagnitute;

	   //Calculate the distance between A and B
	   Distance.x = ( ( (float)b.x ) - ( (float)a.x ) );
	   Distance.y = ( ( (float)b.y ) - ( (float)a.y ) );

	   //Combine both rectangles and half the returned value
	   float XAdd = ( ( (float)b.w ) + ( (float)a.w ) ) / 2.0f;
	   float YAdd = ( ( (float)b.h ) + ( (float)a.h ) ) / 2.0f;

	   //Check if the Distance vector is below 0.0f
	   absDistance.x = ( Distance.x < 0.0f ) ? -Distance.x : Distance.x;
	   absDistance.y = ( Distance.y < 0.0f ) ? -Distance.y : Distance.y;

	   //If the absDistance X is less than X add and the absDistance is less thank YAdd
	   //then it doesn't take a genius to figure out they aren't colliding so return false
	   if( ! ( ( absDistance.x < XAdd ) && ( absDistance.y < YAdd ) ) ) {   
	   return false;
	   }

	    //Get the magnitude by the overlap of the two rectangles
		XMagnitute = XAdd - absDistance.x;
		YMagnitute = YAdd - absDistance.y;

	    //Determine what axis we need to act on based on the overlap
		if( XMagnitute < YMagnitute ) {
		  normal.x = ( Distance.x > 0) ? -XMagnitute : XMagnitute;
		}
		else if ( XMagnitute > YMagnitute ) {
		  normal.y = ( Distance.y > 0) ? -YMagnitute : YMagnitute;
	   }
		return true;
	}

	Rect::operator sf::IntRect() const
	{
		sf::IntRect temp; 

		temp.Left = x; temp.Top = y;
		temp.Right = x+w; temp.Bottom = y+h; 

		return temp;
	}

	Rect::Rect( int X/*=0*/, int Y/*=0*/,int W/*=0*/, int H/*=0*/ ) : 
				x(X),y(Y), w(W), h(H)
	{
	}

	Rect::Rect( const sf::Vector2i& pos, const sf::Vector2i& size )
	{
		x = pos.x; 
		y = pos.y; 
		w = size.x; 
		h = size.y;
	}

	Rect::Rect( const sf::Vector2f& pos, const sf::Vector2f& size )
	{
		x = (int)pos.x; 
		y = (int)pos.y; 
		w = (int)size.x; 
		h = (int)size.y;
	}

	Rect::Rect( const sf::IntRect& rect )
	{
		x = rect.Left; 
		y = rect.Top; 
		w = rect.GetWidth(); 
		h = rect.GetHeight();
	}

	Rect::Rect( const Rect& other )
	{
		x = other.x; y = other.y;
		w = other.w; h = other.h;
	}
	const Rect Rect::operator+( const Rect& other ) const
	{
		return Rect(*this) += other;
	}

	const Rect Rect::operator+( int panning ) const
	{
		return Rect(x-panning,y-panning,w+panning,h+panning);
	}
	Rect& Rect::operator+=( const Rect& other )
	{
		x += other.x; y += other.y;
		w += other.w; h += other.h;
		return *this;
	}

	Rect& Rect::operator-=( const Rect& other ) 
	{
		x -= other.x; y -= other.y;
		w -= other.w; h -= other.h;
		return *this;
	}

	const Rect Rect::operator-( const Rect& other ) const
	{
		return Rect(*this) -= other;
	}

	bool Rect::operator==( const Rect& other ) const
	{
		return (x == other.x && y == other.y &&
				w == other.w && h == other.h);
	}

	bool Rect::operator!=( const Rect& other ) const
	{
		return !(*this == other);
	}

	bool Rect::operator!() const
	{
		return (x == 0 && y == 0 && w == 0 && h == 0);
	}

	Rect& Rect::operator=( const Rect& other )
	{
		x = other.x; y = other.y;
		w = other.w; h = other.h;
		return *this;		
	}

	const Rect Rect::operator-( int panning ) const
	{
		return Rect(x+panning, y+panning,w-panning,h-panning);
	}

	Rect& Rect::operator-=( int panning )
	{
		x += panning; y += panning;
		w -= panning; h -= panning;
		return *this;
	}

	Rect& Rect::operator+=( int panning )
	{
		x -= panning; y -= panning;
		w += panning; h += panning;
		return *this;
	}

	sf::Vector2f Rect::GetPos() const
	{
		return sf::Vector2f((float)x,(float)y);
	}
	
	sf::Vector2i Rect::GetSize() const 
	{
		return sf::Vector2i(w,h);
	}

	void Rect::SetSize( uint32 width, uint32 height )
	{
		w = width; h = height;
	}

	void Rect::SetPos( int32 x, int32 y )
	{
		this->x = x; this->y = y;
	}

	void Functor::operator()()
	{
		std::cout << "Debugging Functor()" << std::endl;
	}

	std::string ToUpper( const std::string& text )
	{
		std::locale loc;
		std::string temp;
		temp.resize(text.size());
		for(uint32 i=0; i<text.size(); i++) {
			temp[i] = std::toupper(text[i],loc);
		}
		return temp;
	}

	sf::Color UnsignedToColor( uint32 rgba )
	{
		uint8 r(0),g(0),b(0),a(0);
		r = uint8(rgba >> 24);
		g = uint8(rgba >> 16);
		b = uint8(rgba >> 8);
		a = uint8(rgba & 0x000000FF);

		return sf::Color(r,g,b,a);
	}

	gui::uint32 ColorToUnsigned( const sf::Color& color )
	{
		uint8 r(0),g(0),b(0),a(0);
		r = color.r;
		b = color.b;
		g = color.g;
		a = color.a;

		uint32 iColor = 0;
		iColor = r << 24 |
				 g << 16 |
				 b << 8  |
				 a;
		return iColor;
	}

	void InsertChar( std::ostream& os, char c, uint32 count )
	{
		for(uint32 i=0; i<count; i++) {
			os << c;
		}
	}

	sf::Shape TwoColoredRectangle( int width, int height, sf::Color upper, sf::Color lower )
	{
		sf::Shape s3;
		//rectangle points
		s3.AddPoint(0.f,0.f,upper);
		s3.AddPoint((float)width,0.f,upper);
		s3.AddPoint((float)width,(float)height,lower);
		s3.AddPoint(0.f,(float)height,lower);

		return s3;
	}

	float ComputeDistance( const sf::Vector2f& first, const sf::Vector2f& second )
	{
		float dist = sqrt(pow(second.x-first.x,2)+pow(second.y-first.y,2));

		return dist;
	}

	void ExtractPath( const std::string& path, std::vector<std::string>& v )
	{
		uint32 pos = 0;
		std::string ptemp = path;
		std::string temp;
		while(ptemp.size()) {
			pos = ptemp.find_first_of('.');
			if(pos == std::string::npos) {
				temp = ptemp; 
				ptemp.clear();
			} else {
				temp = ptemp.substr(0,pos);
				ptemp.erase(0,pos+1);
			}
			v.push_back(temp);
		}
	}
}	//end of namespace gui



std::ostream& operator<<( std::ostream& os, gui::Rect& rect)
{
	os << "X: " << rect.x << " \t\tY: " << rect.y << std::endl
		<< "Width: " << rect.w << " \tHeight: " << rect.h;

	return os;
}

std::stringstream& operator<<( std::stringstream& ss, gui::Rect& rect )
{
	ss << "X: " << rect.x << " \t\tY: " << rect.y << std::endl
		<< "Width: " << rect.w << " \tHeight: " << rect.h;

	return ss;
}
