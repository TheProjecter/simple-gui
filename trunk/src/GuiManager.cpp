#include "../include/gui/GuiManager.hpp"
#include "../include/gui/Widget.hpp"
#include "../include/gui/Debug.hpp"
#include "../include/gui/DefaultFactory.hpp"
#include <tinyxml.h>

#include <iostream>

namespace gui {

	GuiManager::GuiManager( sf::RenderWindow* window ):	m_window(window),
				m_hotSpotX(0),m_hotSpotY(0), m_focus(NULL), 
				m_drag(false),index(0),m_theme(NULL),m_hoverTarget(NULL),
				m_curDrag(NULL)
	{
		m_parser.SetGui(this);
		m_factories.push_back(new DefaultFactory());
		Widget::s_gui = this;
		Mediator::s_currentGui = this;
	}

	GuiManager::GuiManager(): m_window(NULL), m_hotSpotX(0),m_hotSpotY(0),
						m_focus(NULL), m_drag(false),index(0),m_theme(NULL),
						m_hoverTarget(NULL), m_curDrag(NULL)
	{
		m_parser.SetGui(this);
		m_factories.push_back(new DefaultFactory());
		Widget::s_gui = this;
		Mediator::s_currentGui = this;
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
			m_freeWidgets.push_back(widget);
		} else {
			debug_log("Couldn't delete widget \"%s\". Not found!",name.c_str());
		}
	}

	void GuiManager::DeleteWidget( Widget* widget )
	{
		if(!widget) return;

		m_freeWidgets.push_back(widget);
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
		//the focus is the same widget..
		if(m_focus == i->second) return;

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
		m_focus->SetId(i->first);
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

	void GuiManager::StartDrag( Widget* widget, sf::Event* event )
	{
		RegisterDrag(widget->CreateDrag(event));
	}

	void GuiManager::StopDrag(int x, int y)
	{
		if(!m_curDrag) return;

		//only finish the drag if it actually started!
		if(m_curDrag->GetStatus() == Drag::Running) {
			m_curDrag->m_status = Drag::Finished;

			Widget* parent = m_curDrag->GetTargetParent();
			
			//if there is no responsible parent, the gui will take the responsibility
			if(!parent) {

			} else {
				parent->HandleDragStop(m_curDrag);
			}
		}
		delete m_curDrag;
		m_curDrag = NULL;
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
		Mediator::s_currentGui = this;

		//check if the current focus just died
		if(m_focus && m_focus->IsDead()) 
			m_focus = NULL;
		if(m_hoverTarget && m_hoverTarget->IsDead())
			m_hoverTarget = NULL;

		_HandleEvents();

		for(WidgetList::iterator i=m_widgets.begin(); i!= m_widgets.end(); i++) {
			if(i->second->IsDead()){ 
				m_freeWidgets.push_back(i->second);
			} else {
				i->second->Update(diff);
				i->second->Draw();
			}
		}

		//clear the dead widgets
		for(uint32 i=0; i<m_freeWidgets.size(); i++) {
			WidgetList::iterator it = m_widgets.find(m_freeWidgets[i]->GetId());
			if(it != m_widgets.end()) {
				delete it->second;
				m_widgets.erase(it);
			} else {
				debug_log("Couldn't delete top-level widget!");
			}
		}
		m_freeWidgets.clear();
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
		//free the current theme
		if(m_theme) {
			delete m_theme;
			m_theme = NULL;
		}
		m_events.clear();
		m_freeWidgets.clear();
		m_mediator.ClearConnections();
		m_mediator.GetDispatcher().ClearListeners();
		m_mediator.ConsumeEvents();
		m_parser.Parse(&doc);
	}

	void GuiManager::RegisterFactory( AbstractFactory* userFactory )
	{
		m_factories.push_back(userFactory);
	}

	//returns the path in this form: [0]leaf-[1]child-[2]parent
	std::stack<Widget*> GuiManager::QueryWidgetPath( const std::string& path ) const
	{
		std::vector<std::string> temp;
		std::stack<Widget*> stack;
		std::vector<Widget*> helper;
		Widget* current = NULL;

		ExtractPath(path,temp);
		
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
		index = 0;
		m_widgets.clear();
		m_focus = NULL;
		m_hoverTarget = NULL;
		if(m_curDrag) {
			delete m_curDrag;
			m_curDrag = NULL;
		}
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
			Widget* widget = m_freeWidgets[i];
			
			WidgetList::iterator it = m_widgets.find(widget->GetId());
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
					
					//skip dead widgets
					if(currentWidget->IsDead()) continue;

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

						//don't initiate drag, widgets will
 						if(currentWidget->CanDrag(x,y)) {
 							StartDrag(currentWidget,curEvent);
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
					
					this->MoveDrag(curEvent);

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
		if(m_curDrag) 
			delete m_curDrag;

		m_curDrag = drag;
	}

	void GuiManager::MoveDrag( sf::Event* event )
	{
		if(!m_curDrag) return;

		int x = event->MouseMove.X;
		int y = event->MouseMove.Y;

		Rect rect = Rect(x,y,1,1);
		

		//update the position of the drag
		m_curDrag->SetPos(ConvertCoords(x,y));

		//if the start wasn't initiated yet, you may return
		if(m_curDrag->GetStatus() != Drag::Running) return;

		////////////////////////////////////////////////////////////////
		//    Check the current focus target for the new position     //
		////////////////////////////////////////////////////////////////
		
		UpdateDragFocus(x,y);

		/////////////////////////////////////////////////////////////////
		//     Announce the responsible parent that the drag moved     //
		/////////////////////////////////////////////////////////////////
		
		//if there's no parent, the responsible will be the gui mgr
		if(!m_curDrag->GetTargetParent()) {	
			//hardcode the movement.. fix?
			Widget* target = m_curDrag->GetTarget();
			target->SetPosFromDrag(m_curDrag);
		} else {
			Widget* parent = m_curDrag->GetTargetParent();
			parent->HandleDragMove(m_curDrag);
		}
	}

	Widget* GuiManager::GetWidgetAt( const sf::Vector2f& pos ) const
	{
		return GetWidgetAt((int)pos.x, (int)pos.y);
	}

	Widget* GuiManager::GetWidgetAt( int x, int y ) const
	{
		Rect rect = Rect(x,y,1,1);
		for(WidgetList::const_reverse_iterator it = m_widgets.rbegin(); 
			it != m_widgets.rend(); it++) 
		{
			if(it->second->IsCollision(rect)) {
				return it->second;
			}
		}
		return NULL;
	}

	Widget* GuiManager::GetLastWidgetAt( int x, int y ) const
	{
		Widget* current = GetWidgetAt(x,y);
		if(!current) return NULL;
		Rect rect = Rect(x,y,1,1);

		//go to the deepest level of composition possible(last widget colliding)
		while(true) {
			const WidgetList& list = current->GetWidgetList();
			WidgetList::const_iterator i;

			for(i = list.begin(); i != list.end(); i++) {
				//allow hidden widgets!
				if(IsCollision(i->second->m_rect,rect)) {
					current = i->second;
					break;
				}
			}

			//if you got to the end of the list and you didn't collide with anything
			if(i == list.end()) {
				return current;
			}
		}
	}

	void GuiManager::UpdateDragFocus(int x, int y)
	{
		Rect rect = Rect(x,y,1,1);

		//get the lowest-level widget that's colliding with the mouse
		Widget* cur = GetLastWidgetAt(x,y);

		//no widget at that position... null it!
		if(!cur) {
			m_curDrag->m_focusTarget = NULL;
			return;
		}
		//reset the current focus
		m_curDrag->m_focusTarget = NULL;
		std::vector<Widget*> stack = cur->GetWidgetStackPath();

		for(uint32 i=0; i<stack.size(); i++) {
			cur = stack[i];
			//if it is a collision and it accepts the drop 
			if(cur->IsCollision(rect) && cur->AcceptsDrop(m_curDrag)) {
				m_curDrag->m_focusTarget = cur;
				break;
			}
		}

	}

	Widget* GuiManager::QueryWidget( const std::string& path ) const
	{
		std::vector<std::string> temp;
		Widget* current = NULL;

		ExtractPath(path,temp);

		if(temp.size()) {
			current = this->GetWidgetByName(temp[0]);
			if(!current) {
				error_log("Unable to complete query \"%s\". Widget \"%s\" not found!", path.c_str(),temp[0]);
				return NULL;
			}
		} else return NULL;

		for(uint32 i=1; i<temp.size(); i++) {
			current = current->FindChildByName(temp[i]);
			if(!current) {
				error_log("Unable to complete query \"%s\". Widget \"%s\" not found!", path.c_str(),temp[i]);
				return NULL;
			}
		}
		return current;
	}
}