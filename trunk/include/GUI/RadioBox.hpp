#include "Widget.hpp"


namespace gui {

	class RadioBox: public Widget 
	{
	public:
		RadioBox();

		bool IsChecked();
		void Check();
		void UnCheck();
		void SetCheckedImage(sf::Image* image);
		void SetUnCheckedImage(sf::Image* image);
	private:
		bool m_checked;
		mutable sf::Image* m_uncheckedImage;

	protected:
		void Draw() const;
		void OnClickPressed(sf::Event* event);

	};

}