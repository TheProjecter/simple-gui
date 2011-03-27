#include "../include/gui/GuiMgrParser.hpp"
#include "../include/gui/Theme.hpp"
#include "../include/gui/Debug.hpp"
#include "../include/gui/Widget.hpp"
#include "../include/gui/GuiManager.hpp"
#include "../include/gui/AbstractFactory.hpp"

namespace gui
{
	GuiMgrParser::GuiMgrParser( GuiManager* mgr ) : 
				m_gui(mgr),m_loadLayout(false)
	{
		currentListener = "default";
	}

	void GuiMgrParser::IterateTags( TiXmlNode* node )
	{
		if(!node) return;
		XmlParser::IterateTags(node);

		
		if(node->Type() == TiXmlNode::TINYXML_ELEMENT) {
			TiXmlElement* temp = node->ToElement();
			if(strcmp(temp->Value(),"widget") == 0)
			{
				switch(GetTagLocation())
				{
				case None:
					//case </widget> closing a widget.. so pop & compile it
					if(m_widgetInfos.size() && !m_loadLayout) {
						CompileWidget(m_widgetInfos.top());
					} else if(m_loadLayout && m_widgets.size()){
						m_widgets.pop();
					} else {
						error_log("Attempted to pop a widget but stack is empty!");
					}
					break;
				default: break;
				}
			} else if(strcmp(temp->Value(),"listener") == 0) {
				//</listener> not inside a <widget>! so it's a gui listner
				currentListener = "default";

				if(m_tagLoc.size()) 
					m_tagLoc.pop();
			} else if(strcmp(temp->Value(), "commander") == 0) {
				if(m_tagLoc.size()) 
					m_tagLoc.pop();
			} else if(strcmp(temp->Value(),"property") == 0) {
			} else if(strcmp(temp->Value(),"theme") == 0) {
				//do nothing..
			} else {
				//debug_log("Unhandled case: %s", temp->Value()); 
			}
		}
	}

	GuiMgrParser::TagLocation GuiMgrParser::GetTagLocation() const
	{
		if(!m_tagLoc.size()) return None;
		else return m_tagLoc.top();
	}

	void GuiMgrParser::OnXmlElement( TiXmlElement* element )
	{
		if(!element) return;

		TiXmlAttribute* pAttrib = element->FirstAttribute();


		if(strcmp(element->Value(), "listener") == 0)
		{
			std::string name = "default";
			while (pAttrib)
			{
				if(strcmp(pAttrib->Name(),"name") == 0) {
					name = pAttrib->Value();
				} else {
					error_log("Unhandled attribute when loading ui! %s", pAttrib->Name());
				}
				pAttrib = pAttrib->Next();
			}
			currentListener = name;
			m_tagLoc.push(Listener);
		} else if(strcmp(element->Value(),"widget") == 0) {
			switch(GetTagLocation())
			{
			case None:
			{
				Widget* temp = NULL;
				//it's a create widget xml syntax! 
				std::string widgetName; int widgetType = WIDGET;
				while (pAttrib)
				{
					if(strcmp(pAttrib->Name(),"name") == 0)
						widgetName = pAttrib->Value();
					else if(strcmp(pAttrib->Name(),"type") == 0) {
						pAttrib->QueryIntValue(&widgetType);
					} else {
						error_log("Unhandled attribute when loading ui! %s", pAttrib->Name());
					}
					pAttrib = pAttrib->Next();
				}
				if(!m_loadLayout) {
					bool created = false;
					for(uint32 i=0; i<m_gui->m_factories.size(); i++) {
						if(m_gui->m_factories[i]->CanCreateWidget((uint32)widgetType)) {
							temp = m_gui->m_factories[i]->CreateWidget((uint32)widgetType);
							if(temp) {
								created = true;
								temp->SetName(widgetName);
							}
							
							break;						
						}
					}
					if(!created) {
						error_log("No factory was able to create a widget of type: %u!", widgetType);
					} else {
						m_widgetInfos.push(WidgetInfo(temp,(uint32)widgetType));
					}
				} else {
					Widget* temp = NULL;
					if(m_widgets.size()) {
						Widget*	parent = m_widgets.top();
						temp = parent->FindChildByName(widgetName);
						if(!temp) {
							debug_log("Couldn't find widget %s while loading layout!", widgetName.c_str());
						} else {
							m_widgets.push(temp);
						}
					} else {
						Widget* temp = NULL;
						temp = m_gui->GetWidgetByName(widgetName);
						if(!temp) {
							debug_log("Couldn't find widget %s while loading layout!", widgetName.c_str());
						} else {
							m_widgets.push(temp);
						}
					}
				}
			} break;
			case Listener:
			{
				std::string widgetName; int eventType(0);
				while (pAttrib)
				{
					if(strcmp(pAttrib->Name(),"name") == 0)
						widgetName = pAttrib->Value();
					else if(strcmp(pAttrib->Name(),"event") == 0) {
						pAttrib->QueryIntValue(&eventType);
					} else {
						error_log("Unhandled attribute when loading ui! %s", pAttrib->Name());
					}
					pAttrib = pAttrib->Next();
				}
				//if inside a <widget> tag
				if(m_widgetInfos.size()) {
					WidgetInfo& w = m_widgetInfos.top();
					if(w.m_listenerInfo.find(currentListener) != w.m_listenerInfo.end()) {
						ListenerInfo& li = w.m_listenerInfo[currentListener];
						li[eventType].push_back(widgetName);
					} else {
						ListenerInfo& li = w.m_listenerInfo[currentListener];
						li[eventType].push_back(widgetName);
					}
				} else {
					ListenerInfo& li = m_guiInfo.m_listenerInfos[currentListener];
					li[eventType].push_back(widgetName);
				}
			} break;
			default: break;
			} 
		} else if(strcmp(element->Value(),"property") == 0) { //TODO: make settings loading here
			if(!m_loadLayout) {
				if(!m_widgetInfos.size()) {
					debug_log("Tried to load properties, but no widget was found!");
				} else {
					Widget* widget = m_widgetInfos.top().m_widget;
					widget->m_settings.Load(element);
					widget->ReloadSettings();
				}
			} else {
				if(!m_widgets.size()) {
					debug_log("Tried to load properties, but no widget was found!");
				} else {
					m_widgets.top()->m_settings.Load(element);
					m_widgets.top()->ReloadSettings();
				}
			}
		} else if(strcmp(element->Value(),"theme") == 0) {
			std::string themeName;
			while (pAttrib)
			{
				if(strcmp(pAttrib->Name(),"name") == 0) {
					themeName = pAttrib->Value(); 
				} else {
					error_log("Unhandled attribute when loading theme! %s", pAttrib->Name());
				}
				pAttrib = pAttrib->Next();
			}			
			Theme* theme = new Theme(themeName);
			theme->LoadFromXml(element);	//load user-defined data

			m_gui->SetTheme(theme);
		} else {
			//debug_log("Unhandled tag-case %s", element->Value());
		}
	}

	void GuiMgrParser::SetGui( GuiManager* mgr )
	{
		m_gui = mgr;
	}

	void GuiMgrParser::Parse( TiXmlNode* node, bool loadLayout /* = false */)
	{
		m_loadLayout = loadLayout;

		IterateTags(node);

		//compile gui's listeners.. not needed when only loading a layout
		if(!m_loadLayout)
			CompileGui();

		m_guiInfo.m_listenerInfos.clear();
		
		while(m_widgetInfos.size()) {
			m_widgetInfos.pop();
		}
		while(m_tagLoc.size()) {
			m_tagLoc.pop();
		}
		currentListener = "default";
	}

	void GuiMgrParser::ExtractPath( const std::string& t, std::vector<std::string>& v) const
	{
		uint32 pos = 0;
		std::string ptemp = t;
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

	//compiles listener/commander connections inside widgets
	void GuiMgrParser::CompileWidget( WidgetInfo& info )
	{
		if(!info.m_widget) return;

		LInfo linfo = info.m_listenerInfo;

		//add widget to gui
		m_widgetInfos.pop();
		if(m_widgetInfos.size()) {
			Widget* parent = m_widgetInfos.top().m_widget;
			if(!parent) {
				error_log("Couldn't add widget \"%s\"to non-existing parent!",info.m_widget->GetName().c_str());
			} else {
				if(!parent->AddWidget(info.m_widget)) {
					error_log("Couldn't add widget \"%s\" to his parent \"%s\". Maybe duplicate exists?",info.m_widget->GetName().c_str(),parent->GetName().c_str());
				}
			}
		} else {
			
			if(!m_gui->AddWidget(info.m_widget)) {
				error_log("Couldn't add widget \"%s\" to his GUI!. Maybe duplicate exists?",info.m_widget->GetName().c_str());
			}
			
		}
		if(!linfo.size()) return;

		//compile listener connections
		for(LInfo::iterator itr = linfo.begin(); itr != linfo.end(); itr++) 
		{
			for(ListenerInfo::iterator it = itr->second.begin(); it != itr->second.end(); it++) 
			{		
			const std::vector<std::string>& paths = it->second;
			for(uint32 i=0; i<paths.size(); i++) 
			{
				std::string path = paths[i];
				uint32 pos = path.find("this.");

				//build real path
				if(pos != std::string::npos) {
					path.replace(pos,4,info.m_widget->GetWidgetPath());
				} 
				std::stack<Widget*> hierarchy = m_gui->QueryWidgetPath(path);
				Widget* temp = NULL;
				while(hierarchy.size()) {
					temp = hierarchy.top(); 
					hierarchy.pop();
				}
				if(!temp) { 
					error_log("Error compiling widget \"%s\"", info.m_widget->GetWidgetPath());
					return;
				}	
				info.m_widget->m_mediator.Connect(temp,itr->first,it->first);
			}
			}
		}
	}

	void GuiMgrParser::CompileGui()
	{
		//compile listeners
		std::vector<std::string> temp;
		LInfo& linfo = m_guiInfo.m_listenerInfos;

		//compile listener connections
		for(LInfo::iterator itr = linfo.begin(); itr != linfo.end(); itr++) 
		{

			for(ListenerInfo::iterator it = itr->second.begin(); it != itr->second.end(); it++) 
			{
			const std::vector<std::string>& paths = it->second;
			for(uint32 i=0; i<paths.size(); i++) 
			{
				std::string path = paths[i];

				std::stack<Widget*> hierarchy = m_gui->QueryWidgetPath(path);
				Widget* temp = NULL;
				while(hierarchy.size()) {
					temp = hierarchy.top(); 
					hierarchy.pop();
				}
				if(!temp) { 
					error_log("Error compiling widget \"%s\"", path.c_str());
					return;
				}
				m_gui->m_mediator.Connect(temp,itr->first,it->first);
			}
		}
		}
	}

	GuiMgrParser::WidgetInfo::WidgetInfo( Widget* widget /*=NULL*/, 
										  uint32 type /*= WIDGET*/ ) 
		: m_type(type), m_widget(widget)
	{

	}

}