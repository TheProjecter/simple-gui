#pragma once

#include "XmlParser.hpp"
#include <stack>
#include <utility>
#include "Defines.hpp"
#include <tinyxml.h>
#include "Settings.hpp"

namespace gui
{
	class GuiManager;
	class Widget;

	class GuiMgrParser : public XmlParser
	{
	public:
		GuiMgrParser(GuiManager* mgr=NULL);
		void SetGui(GuiManager* mgr);
		void Parse(TiXmlNode* node, bool loadLayout = false);
		void ExtractPath(const std::string&, std::vector<std::string>&) const;
	protected:
		void IterateTags(TiXmlNode* node);
		void OnXmlElement(TiXmlElement* node);

	private:
		enum TagLocation {
			None,
			Listener,
		};

		GuiManager* m_gui;
		std::stack<TagLocation> m_tagLoc;
		TagLocation GetTagLocation() const;

		typedef std::map<uint32, std::vector<std::string> > ListenerInfo;
		typedef std::map<std::string, ListenerInfo > LInfo;
		
		struct WidgetInfo {
			WidgetInfo(Widget* widget = NULL, uint32 type = WIDGET);
			uint32 m_type;
			Widget* m_widget;
			LInfo m_listenerInfo;
		};
		struct GuiInfo {
			LInfo m_listenerInfos;
		};
		mutable std::stack<WidgetInfo> m_widgetInfos;	//used for ui loading
		mutable std::stack<Widget*> m_widgets;			//used for layout loading
		GuiInfo m_guiInfo;

		void CompileWidget(WidgetInfo& info);
		void CompileGui();

		std::string currentListener;
		bool m_loadLayout;
	};
}
