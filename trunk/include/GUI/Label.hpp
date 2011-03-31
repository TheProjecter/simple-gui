#pragma once

#include "Widget.hpp"


namespace gui {

	class Label: public Widget {
	public:
		Label();
		void SetPos(int x, int y, bool forceMove = false);
		void SetAlignment(Alignment a);

	private:
		void Draw() const;
		Alignment alignment;
	protected:
		sf::String text;
		void Update(float diff);

	};
}
