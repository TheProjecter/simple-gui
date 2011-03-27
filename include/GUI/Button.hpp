#pragma once

#include "Widget.hpp"

namespace gui {

	class Button: public Widget 
	{
	public:
		Button();
		void SetText(const std::string& text);
		void SetPos(int x, int y, bool forceMove = false);
		
	protected:
		virtual void Draw() const;
		void OnClickPressed(sf::Event* event);
		void OnClickReleased(sf::Event* event);

		virtual void SetInactiveImage();
		virtual void SetActiveImage();
		virtual void InitGraphics();

		bool m_active;
		sf::String m_text;
	};	

	class ImageButton : public Button 
	{
	public:
		ImageButton();

	protected:
		void Draw() const;
		void SetInactiveImage();
		void SetActiveImage();
		void InitGraphics();
	};

}