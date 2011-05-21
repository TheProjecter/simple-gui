#pragma once

#include "Widget.hpp"


namespace gui {

	class Label: public Widget {
	public:
		Label();
		void SetPos(int x, int y, bool forceMove = false, bool save =true);
		void SetAlignment(Alignment a);
		
		void SetText(const std::string& text);
		std::string GetText() const;
	private:
		void Draw() const;
		Alignment alignment;
	protected:
		sf::String m_text;
		void Update(float diff);
		virtual void ReloadSettings();

	};
}
