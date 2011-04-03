#pragma once

#include "Widget.hpp"

namespace gui
{
	class GuiManager;

	class UiEditor : public Widget
	{
	public:
		UiEditor(GuiManager* gui);
		~UiEditor();
	protected:
	private:
	};
}