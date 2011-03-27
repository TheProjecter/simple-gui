#include "../include/gui/GuiManager.hpp"
#include "../include/gui/Widget.hpp"
#include "../include/gui/Debug.hpp"
#include "../include/gui/DefaultFactory.hpp"
#include <tinyxml.h>

#include <iostream>

namespace gui {

	GuiManager::GuiManager( sf::RenderWindow* window ):	m_window(window),
				m_hotSpotX(0),m_hotSpotY(0), m_focus(NULL), 
				m_drag(false),index(0),m_theme(NULL),m_hoverTarget(NULL)
	{
		debug_log("Creating manager!");
		m_parser.SetGui(this);
		m_factories.push_back(new DefaultFactory());

	}

	GuiManager::GuiManager(): m_window(NULL), m_hotSpotX(0),m_hotSpotY(0),
						m_focus(NULL), m_drag(false),index(0),m_theme(NULL),
						m_hoverTarget(NULL)
	{
		m_parser.SetGui(this);
		m_factories.push_back(new DefaultFactory());
	}
	GuiManager::~GuiManager()
	{
		ClearWidgets();

		for(uint32 i=0; i<m_factories.size(); i++) {
			delete m_factories[i];
		}
		m_factories.clear();
	}

	void GuiManager::DeleteWidget( const std::string& name )
	{
		//TODO: reduce the O(n) + O(log(n)) complexity
		if(Widget* widget = GetWidgetByName(name)) {
			uint32 id = widget->GetId();
			WidgetList::iterator it = m_widgets.find(id);
			if(it != m_widgets.end()) {
				delete widget;
				m_widgets.erase(it);
			} else std::cout << "Unable to delete widget !" << name.c_str() << std::endl;
		}
	}

	void GuiManager::DeleteWidget( Widget* widget )
	{
		if(!widget) return;

		m_freeWidgets.push_back(widget->GetId());
	}

	//widgets must be dynamically allocated!
	bool GuiManager::AddWidget( Widget* widget )
	{
		//don't allow duplicate widgets!
		if(GetWidgetByName(widget->GetName())) {
			error_log("Couldn't create widget named %s. Duplicate exists!",widget->GetName());
			return false;
		}
		
		widget->SetId(++index);
		m_widgets[index] = widget;

		return true;
	}

	void GuiManager::SetHasFocus( WidgetList::reverse_iterator& i )
	{
		//if there was another focus before...he lost focus
		if(m_focus) {
			m_focus->m_isFocus = false;
			m_focus->_HandleOnFocusLost();
		}
		Widget* widget = i->second;		
		i++;
		WidgetList::iterator itr = i.base();
		m_widgets.erase(itr);
		m_widgets[++index] = widget;
		i = m_widgets.rbegin();
		m_focus = widget;
		m_focus->m_isFocus = true;
		m_focus->OnFocus();
	}

	Widget* GuiManager::GetWidgetByName( const std::string& name ) const
	{
		//TODO: Expensive O(n) .. fix it!
		for(WidgetList::const_iterator itr = m_widgets.begin(); 
			itr != m_widgets.end(); itr++) 
		{
			if(itr->second->GetName() == name) {
				return itr->second;
			}
		}
		return NULL;
	}

	void GuiManager::StartDrag(int hotSpotX, int hotSpotY)
	{
		m_hotSpotX = hotSpotX; 
		m_hotSpotY = hotSpotY;
		m_drag = true;
	}

	void GuiManager::StopDrag(int x, int y)
	{
		m_drag = false;
	}

	std::vector<Widget*> GuiManager::GetWidgetsByType(WidgetType type) const
	{
		std::vector<Widget*> chosen;
		for(WidgetList::const_iterator i = m_widgets.begin(); i != m_widgets.end(); i++){
			if(i->second->GetType() == type)
				chosen.push_back(i->second);
		}
		return chosen;
	}

	void GuiManager::Update(float diff)
	{
		if(!m_window) {
			debug_log("GuiManager doesn't not have a render window!");
			return;
		}

		//set the static gui pointer to the current mgr, since there could be more
		Widget::s_gui = this;

		_HandleEvents();

		for(WidgetList::iterator i=m_widgets.begin(); i!= m_widgets.end(); i++) {
			i->second->Update(diff);
			i->second->Draw();
		}
	
	}

	void GuiManager::RegisterEvent( sf::Event* event )
	{
		m_events.push_back(event);
	}

	void GuiManager::SetWindow( sf::RenderWindow* window )
	{
		m_window = window;
	}

	void GuiManager::SetTheme( Theme* theme )
	{
		m_theme = theme;

		for(WidgetList::iterator it	= m_widgets.begin(); it != m_widgets.end(); it++) {
			it->second->ReloadTheme();
		}
	}

	Theme* GuiManager::GetTheme() const
	{
		return m_theme;
	}

	sf::Vector2f GuiManager::ConvertCoords( int x, int y )
	{
		return m_window->ConvertCoords(x,y);
	}

	//TODO: update this to work with tinyxml!!
	bool GuiManager::LoadLayout( const char* filename )
	{
		if(!filename) return false;

		TiXmlDocument doc;

		if(!doc.LoadFile(filename)) {
			debug_log("Unable to load layout \"%s\", because: \"%s\"", filename,doc.ErrorDesc());
			return false;
		}
		//GuiMgr Parse should be able to handle Layout loading.. it's just like a .ui only with less features!
		m_parser.Parse(&doc,true);
		
		return true;
	}

	void GuiManager::SaveLayout( const char* filename )
	{
		TiXmlDocument doc;

		TiXmlElement* elem = NULL;

		if(m_theme) {
			elem = new TiXmlElement("theme");
			elem->SetAttribute("name",m_theme->GetFilePath().c_str());
			doc.LinkEndChild(elem);
		}

		for(WidgetList::iterator it = m_widgets.begin(); it != m_widgets.end(); it++) {
			it->second->SaveLayout(&doc);
		}
		doc.SaveFile(filename);
	}

	void GuiManager::LoadUI( const char* filename )
	{
		TiXmlDocument doc;
		if(!doc.LoadFile(filename)) {
			error_log("Couldn't load %s.ui, because: \"%s\"",filename, doc.ErrorDesc());
			return;
		}
		ClearWidgets();
		m_mediator.ClearConnections();
		m_parser.Parse(&doc);
	}

	void GuiManager::RegisterFactory( AbstractFactory* userFactory )
	{
		m_factories.push_back(userFactory);
	}

	std::stack<Widget*> GuiManager::QueryWidgetPath( const std::string& path ) const
	{
		std::vector<std::string> temp;
		std::stack<Widget*> stack;
		std::vector<Widget*> helper;
		Widget* current = NULL;

		m_parser.ExtractPath(path,temp);
		
		if(temp.size()) {
			current = this->GetWidgetByName(temp[0]);
			if(!current) {
				error_log("Unable to complete query \"%s\". Widget \"%s\" not found!", path.c_str(),temp[0]);
				return stack;
			}
		} else return stack;

		helper.push_back(current);

		for(uint32 i=1; i<temp.size(); i++) {
			current = current->FindChildByName(temp[i]);
			if(!current) {
				error_log("Unable to complete query \"%s\". Widget \"%s\" not found!", path.c_str(),temp[i]);
				return stack;
			}
			
			helper.push_back(current);
		}

		for(int i=helper.size()-1; i>=0; i--) {
			stack.push(helper[i]);
		}

		return stack;
	}

	void GuiManager::ClearWidgets()
	{
		for(WidgetList::iterator it = m_widgets.begin(); it != m_widgets.end(); it++) {
			Widget* widget = it->second;
			delete widget;
		}
		m_widgets.clear();
		m_focus = NULL;
		m_hoverTarget = NULL;
	}

	void GuiManager::SaveUI( const char* filename )
	{
		TiXmlDocument doc;
		
		//save current theme
		if(m_theme) {
			TiXmlElement* e = new TiXmlElement("theme");
			e->SetAttribute("name", m_theme->GetFilePath().c_str());
			m_theme->SaveToXml(e);
			doc.LinkEndChild(e);
		}

		TiXmlComment* c = new TiXmlComment("External listeners. Accessible from the Gui Manager");
		doc.LinkEndChild(c);

		//save listeners
		m_mediator.SaveConnections(&doc);

		if(m_widgets.size()) {
			c = new TiXmlComment("main widgets:");
			doc.LinkEndChild(c);
		}

		//save widgets
		for(WidgetList::iterator it = m_widgets.begin(); it!=m_widgets.end(); it++) {
			it->second->SaveUI(&doc);
		}
		doc.SaveFile(filename);
	}

	void GuiManager::FreeWidgets()
	{
		for(uint32 i=0; i<m_freeWidgets.size(); i++) {
			uint32 id = m_freeWidgets[i];
			
			WidgetList::iterator it = m_widgets.find(id);
			if(it != m_widgets.end()) {
				delete it->second;
				m_widgets.erase(it);
			}
		}
	}

	void GuiManager::_HandleEvents()
	{
		for(int i=0; i<(int)m_events.size(); i++) {
			sf::Event* curEvent = m_events[i];
			WidgetList::reverse_iterator itr;
			switch(curEvent->Type) {
			case sf::Event::MouseButtonPressed:
				for(itr=m_widgets.rbegin(); itr!=m_widgets.rend();itr++) {
					Widget* currentWidget = itr->second;
					int x = curEvent->MouseButton.X;
					int y = curEvent->MouseButton.Y;
					sf::Vector2f c = m_window->ConvertCoords(x,y);
					x = (int)c.x;
					y = (int)c.y;
					if(currentWidget->IsCollision(Rect(x,y,1,1)) && currentWidget->m_visible) 
					{
						currentWidget->RegisterEvent(curEvent);
						SetHasFocus(itr);
						int X = (int)currentWidget->GetPos().x;
						int Y = (int)currentWidget->GetPos().y;

						//don't drag the widget if the cursor collided with one of it's widgets
						if(currentWidget->CanDrag(x,y)) {
							StartDrag(x-X,y-Y);
						}
						break;
					}
				}
				//if a click happened && no widgets were clicked... there's no focus
				if(itr == m_widgets.rend()) {
					if(m_focus) {
						m_focus->_LoseFocus();

						m_focus = NULL;
					}
				}
				break;
			case sf::Event::MouseButtonReleased:
				{
					sf::Vector2f a = m_window->ConvertCoords(curEvent->MouseButton.X,curEvent->MouseButton.Y);
					StopDrag((int)a.x,(int)a.y);
					if(m_focus) {
						//focus will received the MouseReleased event even if it's not currently colliding!
						m_focus->RegisterEvent(curEvent);

						//if you're not colliding though.. you will lose focus!
						if(!m_focus->IsCollision(Rect((int)a.x,(int)a.y,1,1))) {
							m_focus->_LoseFocus(false);
							m_focus = NULL;
						}
					}
					break;
				}
			case sf::Event::MouseMoved:
				{				
					sf::Vector2f a = m_window->ConvertCoords(curEvent->MouseMove.X,curEvent->MouseMove.Y);
					if(m_drag && m_focus) {

						m_focus->SetPos((int)a.x-m_hotSpotX, (int)a.y-m_hotSpotY);
					}
					if(m_focus) 
						m_focus->RegisterEvent(curEvent);
					//check for hovering events, they're in order by focus levels, 
					//so I'll get the widgets in front first
					Widget* temp = NULL;
					WidgetList::reverse_iterator it;
					for(it = m_widgets.rbegin(); it != m_widgets.rend(); it++) {
						if(!it->second->m_visible) continue;

						if(it->second->IsCollision(Rect((int)a.x,(int)a.y,1,1)) ) {

							//if it's the same widget.. don't count it
							if(m_hoverTarget == it->second) {
								m_hoverTarget->RegisterEvent(curEvent);
								break;
							}

							it->second->OnHover();	

							temp = m_hoverTarget;
							m_hoverTarget = it->second;
							m_hoverTarget->RegisterEvent(curEvent);

							break; //the mouse can only hover over 1 widget at a time..
						}
					}
					//if hoverTarget was lost
					if(temp) {
						temp->OnHoverLost();
					} else if(it == m_widgets.rend()) {
						if(m_hoverTarget) m_hoverTarget->OnHoverLost();
						m_hoverTarget = NULL;
					}

				} break;
			case sf::Event::KeyPressed:
			case sf::Event::KeyReleased:
				if(m_focus) m_focus->RegisterEvent(curEvent);
				break;
			default: if(m_focus) m_focus->RegisterEvent(curEvent);
			}
		}

		//we solved the events
		m_events.clear();

	}

	sf::RenderWindow* GuiManager::GetWindow() const
	{
		return m_window;
	}

	void GuiManager::RegisterDrag( Drag* drag )
	{
		m_curDrag = drag;
	}
}