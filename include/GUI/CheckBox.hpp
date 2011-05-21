#pragma once

#include "Widget.hpp"

namespace gui {

	class CheckBox: public Widget {
	public:
		CheckBox();

		bool IsChecked() const;
		void Check();
		void UnCheck();
		void SetCheckedImage(sf::Image* image);
		void SetUnCheckedImage(sf::Image* image);
	private:
		bool m_checked;
		sf::Image* m_uncheckedImage;


	protected:
		virtual void ReloadSettings();
		void Draw();
		void OnClickPressed(sf::Event* event) { m_checked = !m_checked; }

	};

}