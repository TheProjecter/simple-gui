#include "Widget.hpp"

namespace gui 
{
	class Slider: public Widget
	{
		public:
			Slider();
			Slider(int upper, int lower);
			~Slider() {}
			void SetCursorImage(sf::Image* image);
			void SetLimits(int lower, int upper);
			void SetPos(int x, int y, bool forceMove = false);
			int GetValue();
			void SetValue(int value);
			void Resize(int w, int h, bool save = true);
		private:
			int m_curValue;
			int m_upperLimit, m_lowerLimit;
			sf::Image* m_cursor;
			Rect m_cursorRect;
			bool m_sliderDrag;					//drag flag
			int m_hotSpotX, m_hotSpotY;			//used for drag
		protected:
			void CalculateSliderPos();
			void Update(float diff);
			void Draw() const;

			//event handling functions
			virtual void OnClickPressed(sf::Event* event);
			virtual void OnClickReleased(sf::Event* event);
			virtual void OnOtherEvents(sf::Event* event);
	};
}
