#include "../include/gui/Widget.hpp"
#include "../include/gui/Event.hpp"
#include "../include/gui/GuiManager.hpp"
#include <iostream>
#include <sstream>
#include <stack>
#include <tinyxml.h>

namespace gui {

	GuiManager* Widget::s_gui = NULL;

	Widget::Widget() : index(0), m_parent(0), m_focus(0),m_movable(true),
					m_visible(true),m_mainVisible(true),m_clicked(false),
					m_released(false), m_type(WIDGET), m_drag(false),
					m_hotSpotX(0), m_hotSpotY(0), m_needUpdate(true),
					m_resizable(true),m_isFocus(false),m_transparency(200),
					m_individualTheme(false), m_hovering(false),
					m_hoverTarget(NULL), m_solid(false), m_allowSave(true),
					m_sprite(NULL),m_dropFlags(Drag::WidgetOnly),
					m_dead(false),m_loading(false),m_doubleClickDiff(0),
					m_doubleClickActivated(false), m_doubleClickTime(500)
	{
		m_mediator.SetCurrentPath(m_name);

		//set theme/default colors
		if(s_gui->GetTheme() && s_gui->GetTheme()->HasProperty("widget_background_color"))
			m_shape.SetColor(s_gui->GetTheme()->GetColor("widget_background_color"));
		else m_shape.SetColor(sf::Color(241,213,231));


		//buttons particular size hint
		m_sizeHint.x = 400;
		m_sizeHint.y = 300;

		//buttons particular size policy
		m_horizontalPolicy	= Fixed;
		m_verticalPolicy	= Fixed;	
	}

	Widget::Widget( const std::string& name ): index(0), m_parent(0), 
		m_focus(0),m_movable(true),m_visible(true),m_mainVisible(true),
		m_clicked(false),m_released(false), m_type(WIDGET), m_drag(false),
		m_hotSpotX(0), m_hotSpotY(0), m_needUpdate(true),m_resizable(true),
		m_isFocus(true),m_transparency(200), m_individualTheme(false),
		m_hovering(true),m_hoverTarget(NULL), m_solid(false), 
		m_allowSave(true), m_sprite(NULL),m_dropFlags(Drag::WidgetOnly),
		m_dead(false),m_loading(false),m_doubleClickDiff(0),
		m_doubleClickActivated(false), m_doubleClickTime(500)
	{
		SetName(name);
		m_mediator.SetCurrentPath(name);

		//set theme/default colors
		if(s_gui->GetTheme() && s_gui->GetTheme()->HasProperty("widget_background_color"))
			m_shape.SetColor(s_gui->GetTheme()->GetColor("widget_background_color"));
		else m_shape.SetColor(sf::Color(241,213,231));

		//buttons particular size hint
		m_sizeHint.x = 400;
		m_sizeHint.y = 300;

		//buttons particular size policy
		m_horizontalPolicy	= Fixed;
		m_verticalPolicy	= Fixed;	
	}
	Widget::~Widget()
	{
		OnDestroy();

		for(WidgetList::iterator it = m_widgets.begin(); 
			it!= m_widgets.end();it++) 
		{
			delete it->second;
		}
		m_widgets.clear();
		m_mediator.ClearConnections();
		m_mediator.ConsumeEvents();
	}

	void Widget::SetMovable( bool flag )
	{
		m_movable = flag;
	}

	void Widget::Show()
	{
		OnShow();
		m_settings.SetStringValue("visibility", "on");
		m_visible = true; 
		ShowChildren();
	}

	void Widget::Hide()
	{
		OnHide();
		m_settings.SetStringValue("visibility", "off");
		m_visible = false; 
		HideChildren();
	}

	bool Widget::Clicked() const
	{
		return m_clicked;
	}

	//if the widget has a parent.. the coordinates are applied withing the parent
	void Widget::Move( int x, int y )
	{
		if(!m_parent)
			SetPos(m_rect.x+x,m_rect.y+y);
		else SetPos(m_parent->GetRect().x+x,m_parent->GetRect().y+y);
	}

	Widget::WidgetList& Widget::GetWidgetList() const
	{
		return m_widgets;
	}

	void Widget::SetClicked( bool flag )
	{
		m_clicked = flag; 
	}

	//these seem pretty useless...
	void Widget::ShowBackground()
	{
		m_mainVisible = true;
	}

	void Widget::HideBackground()
	{
		m_mainVisible = false;
	}

	gui::uint32 Widget::GetType() const
	{
		return m_type;
	}

	void Widget::SetType( uint32 type )
	{
		m_type = type;
	}

	void Widget::SetParent( Widget* parent )
	{
		m_parent = parent;

		m_mediator.SetCurrentPath(GetWidgetPath());

		//error checking for positioning since you now got a new parent
		SetPos(m_rect.x, m_rect.y,true);
	}

	Widget* Widget::GetParent() const
	{
		return m_parent;
	}

	void Widget::SetBackgroundColor( sf::Color color )
	{
		m_shape.SetColor(color);
		m_settings.SetUint32Value("background-color",ColorToUnsigned(color));
		m_individualTheme = true;
	}

	void Widget::Resize( int w, int h , bool save /*=true*/)
	{
		Rect temp = m_rect;
		m_rect.w = w; 
		m_rect.h = h; 

		sf::Color color(255,255,255);

		if(!m_sprite) {
			m_shape = sf::Shape::Rectangle(sf::Vector2f(0,0),sf::Vector2f((float)w,(float)h),color);
			m_shape.SetPosition(GetPos());
		} else {
			m_sprite->Resize((float)m_rect.w, (float)m_rect.h);
		}
		if(save) {
			m_settings.SetUint32Value("width", m_rect.w);
			m_settings.SetUint32Value("height", m_rect.h);
		}
		OnResize(m_rect);
		InitGraphics();
		UpdateClipArea();
	}

	void Widget::SetPos( int x, int y, bool forceMove, /* = false */
						 bool save /*=true*/ )
	{
		if(!m_movable && !forceMove) return;

		m_needUpdate = true;

		Rect temp = m_rect;
		m_rect.x = x; 
		m_rect.y = y;

		//error checking to keep widgets in place, to allow outside 
		//of parents rect, modify the drag flags to anywhere?
		if(m_parent && m_dropFlags == Drag::WidgetOnly ) 
		{
			const Rect& prect = m_parent->GetRect();
			const WidgetList& widgets = m_parent->GetWidgetList();

			//don't allowed then leave the parent's rect
			if(m_rect.x < prect.x)
				m_rect.x = prect.x;
			if(m_rect.y < prect.y)
				m_rect.y = prect.y;

// 			//child widgets CAN? be bigger than parent widgets
// 			if(m_rect.w > prect.w) 
// 				m_rect.w = prect.w;
// 			if(m_rect.h > prect.h) 
// 				m_rect.h = prect.h;

			//don't allowed then leave the parent's rect
			if(m_rect.x+m_rect.w > prect.x+prect.w) {
				m_rect.x = prect.x + prect.w - m_rect.w;
			}
			if(m_rect.y+m_rect.h > prect.y+prect.h) {
				m_rect.y = prect.y + prect.h - m_rect.h;
			}	
		}
		//also move child widgets
		for(WidgetList::iterator it = m_widgets.begin(); it != m_widgets.end(); it++) {
			Widget* widget = it->second;
			Rect childRect = widget->GetRect();
			int xpos = m_rect.x - temp.x + childRect.x;
			int ypos = m_rect.y - temp.y + childRect.y ;
			widget->SetPos(xpos,ypos,true); //force child widgets to move!
		}
		ResolveChildCollisions();	//this is slightly broken!

		if(save) {
			m_settings.SetInt32Value("posx",m_rect.x);
			m_settings.SetInt32Value("posy",m_rect.y);
		}

		if(!m_sprite)
			m_shape.SetPosition(m_rect.GetPos());
		else m_sprite->SetPosition(m_rect.GetPos());

		OnMove(temp);
		UpdateClipArea();
	}

	void Widget::SetPos( const sf::Vector2f& pos, bool forceMove, /*= false*/
						 bool save /*=true*/)
	{
		SetPos((int)pos.x,(int)pos.y,forceMove,save);
	}

	const Rect& Widget::GetRect() const
	{
		return m_rect;
	}

	sf::Vector2f Widget::GetPos() const
	{
		return sf::Vector2f((float)m_rect.x,(float)m_rect.y);
	}

	bool Widget::AddWidget( Widget* child )
	{
		if(!child) return false;
		if(FindChildByName(child->GetName())) {
			debug_log("Couldn't add widget \"%s\" to widget: \"%s\"",child->GetName().c_str(),m_name.c_str());		
			return false;
		}

		child->SetParent(this);
		child->SetId(++index);

		m_widgets[index] = child;
		return true;
	}

	void Widget::DeleteWidget( const std::string& widgetName )
	{
		//O(n) search complexity
		if(Widget* child = FindChildByName(widgetName)) {
			//another O(log(n)) search complexity.. need to improve
			WidgetList::iterator it = m_widgets.find(child->GetId());

			if(it != m_widgets.end()) {
				m_freeWidgets.push_back(child);
			}
		}
	}

	//might remove another widget if passed a widget from another parent
	//which happens to have a id that matches one from the current widget
	bool Widget::RemoveWidget( Widget* widget )
	{
		if(!widget) return false;

		WidgetList::iterator it = m_widgets.find(widget->GetId());
		if(it == m_widgets.end()) {
			debug_log("Widget \"%s\" doesn't have a child called \"%s\"", 
						m_name.c_str(),widget->GetName().c_str());		
			return false;
		}
		m_widgets.erase(it);
		return true;
	}

	Widget* Widget::FindChildByName( const std::string& name ) const
	{
		for(WidgetList::const_iterator it = m_widgets.begin(); 
			it != m_widgets.end(); it++) 
		{
			if(it->second->GetName() == name) {
				return it->second;
			}
		}
		return NULL;
	}

	gui::uint32 Widget::GetId() const
	{
		return m_id;
	}

	void Widget::SetId( uint32 id )
	{
		m_id = id;
	}

	void Widget::OnClickPressed( sf::Event* event )
	{
		SetClicked(true); 
		//std::cout << GetName() << ": Click Pressed" << std::endl;
		m_mediator.PostEvent(new gui::OnClickPressed(this));
	}

	void Widget::OnClickReleased( sf::Event* event )
	{
		SetClicked(false); 
		//std::cout << GetName() << ": Click Released" << std::endl;
		m_mediator.PostEvent(new gui::OnClickReleased(this));
	}

	void Widget::OnKeyPressed( sf::Event* event )
	{
		m_mediator.PostEvent(new gui::OnKeyPressed(this,event->Key));
		//std::cout << GetName() << ": Key Pressed" << std::endl;
	}

	void Widget::OnKeyReleased( sf::Event* event )
	{
		m_mediator.PostEvent(new gui::OnKeyReleased(this,event->Key));
		//std::cout << GetName() << ": Key Released" << std::endl;
	}

	void Widget::OnOtherEvents( sf::Event* event )
	{
		//std::cout << GetName() << ": Other Event! " << event->Type << std::endl;
	}

	void Widget::OnTextEntered( sf::Event* event )
	{
		//std::cout << GetName() << ": TextEntered! : " << (int)event->Text.Unicode << std::endl;
	}

	void Widget::SetName( const std::string& newName )
	{
		m_name = newName;
		m_mediator.SetCurrentPath(GetWidgetPath());
	}

	const std::string& Widget::GetName() const
	{
		return m_name;
	}

	void Widget::Update( float diff )
	{
		if(m_dead) return;

		//check if the current focus just died
		if(m_focus && m_focus->m_dead) 
			m_focus = NULL;
		if(m_hoverTarget && m_hoverTarget->m_dead)
			m_hoverTarget = NULL;

		if(m_doubleClickActivated) {
			m_doubleClickDiff -= diff;
		}

		//handle the events..also takes care of child widget events
		_HandleEvents();

		for(WidgetList::iterator it=m_widgets.begin(); it!=m_widgets.end();it++) {
			if(it->second->m_dead) {
				m_freeWidgets.push_back(it->second);
			} else it->second->Update(diff);
		}

		//free the dead widgets
		for(uint32 i=0;i<m_freeWidgets.size(); i++) {
			uint32 guid = m_freeWidgets[i]->GetId();
			WidgetList::iterator it = m_widgets.find(guid);
			if(it != m_widgets.end()) {
				Widget* widget = it->second;

				//if the dying widget contains the current focus.. null it!
				if(widget->ContainsWidget(m_focus)) {
					m_focus = NULL;
				} 
				if(widget->ContainsWidget(m_hoverTarget)) {
					m_hoverTarget = NULL;
				}
				delete it->second;
				m_widgets.erase(it);
			} else {
				debug_log("Couldn't find widget \"%s\" for some reason.",m_freeWidgets[i]->GetName());
			}
		}

		m_freeWidgets.clear();
		//update animations if needed, etc
	}

	void Widget::OnChildEvent( Widget* child, sf::Event* event )
	{
// 		std::cout << "Widget::OnChildEvent(): Me(" << m_name << ") Child("
// 				  << child->GetName() << std::endl;

		//Send the events where they're supposed to go
		switch(event->Type) {
			case sf::Event::MouseButtonPressed: 
				OnChildClickPressed(child,event);	
				break;
			case sf::Event::MouseButtonReleased:
				OnChildClickReleased(child,event); 
				break;
			case sf::Event::KeyPressed:			
				OnChildKeyPressed(child,event);	
				break;
			case sf::Event::KeyReleased:		
				OnChildKeyReleased(child,event);	
				break;
			case sf::Event::TextEntered:		
				OnChildTextEntered(child,event);	
				break;
			default:							
				OnChildOtherEvents(child,event);	
				break;
		}
	}

	void Widget::OnChildClickPressed( Widget* child,sf::Event* event )
	{

	}

	void Widget::OnChildClickReleased( Widget* child,sf::Event* event )
	{

	}

	void Widget::OnChildKeyPressed( Widget* child, sf::Event* event )
	{

	}

	void Widget::OnChildKeyReleased( Widget* child, sf::Event* event )
	{

	}

	void Widget::OnChildTextEntered( Widget* child, sf::Event* event )
	{

	}

	void Widget::OnChildOtherEvents( Widget* child, sf::Event* event )
	{

	}

	void Widget::Draw() const
	{
		if(!m_visible)
			return;

		s_gui->GetWindow().Draw(m_shape);

		//also draw children if any
		for(WidgetList::const_iterator it = m_widgets.begin(); it != m_widgets.end(); it++) {
			//don't draw outside parent's rect
			StartClipping();
				it->second->Draw();
			StopClipping();
		}
	}


	//deprecated?
	void Widget::Draw( const sf::Image* image )
	{
		if(!image) return;
		
		sf::Sprite sprite;
		sprite.SetImage(*image);
		sprite.SetPosition(GetPos());
		sprite.Resize(GetSize());
		sprite.SetColor(sf::Color(255,255,255,m_transparency));
		s_gui->GetWindow().Draw(sprite);

	}
	sf::Vector2f Widget::GetSize() const
	{
		return sf::Vector2f((float)m_rect.w,(float)m_rect.h);
	}

	void Widget::ConvertCoords( sf::Vector2f& coords )
	{
		coords = s_gui->GetWindow().ConvertCoords((int)coords.x,(int)coords.y);
	}

	void Widget::HideChildren()
	{
		//post a OnHide event
		OnHide();

		//make your children post the same event, since they're invisble too now
		for(uint32 i=0; i<m_widgets.size(); i++) {
			m_widgets[i]->HideChildren();
		}
	}

	void Widget::ShowChildren()
	{
		OnShow();

		for(uint32 i=0; i<m_widgets.size(); i++) {
			m_widgets[i]->ShowChildren();
		}
	}

	void Widget::RegisterEvent( sf::Event* event )
	{
		m_events.push_back(event);
	}

	void Widget::_DispatchEvent( sf::Event* event )
	{
		OnEvent(event);
	}
	
	void Widget::OnEvent( sf::Event* event )
	{
		//std::cout << "Widget::OnEvent()! " << m_name << std::endl;
		switch(event->Type) 
		{
			case sf::Event::MouseButtonPressed: OnClickPressed(event);	break;
			case sf::Event::MouseButtonReleased:OnClickReleased(event); break;
			case sf::Event::KeyPressed:			OnKeyPressed(event);	break;
			case sf::Event::KeyReleased:		OnKeyReleased(event);	break;
			case sf::Event::TextEntered:		OnTextEntered(event);	break;
			default:							OnOtherEvents(event);	break;
		}
	}

	//deprecated
	void Widget::_StartDrag( int x, int y )
	{
		m_drag = true;
		m_hotSpotX = x; 
		m_hotSpotY = y;
	}

	bool Widget::HasWidgets() const
	{
		return !m_widgets.empty();
	}

	bool Widget::CanDrag(int x, int y) const
	{
		if(!m_movable) return false;

		for(WidgetList::iterator it = m_widgets.begin(); it != m_widgets.end(); it++) {
			if(it->second->IsCollision(Rect(x,y,1,1))) {
				return false;
			}
		}
		return true;
	}

	gui::uint8 Widget::GetTransparency() const
	{
		return m_transparency;
	}

	void Widget::SetTransparency( uint8 val )
	{
		m_transparency = val;
		m_settings.SetUint32Value("alpha", (uint32)val);
	}

	void Widget::_HandleEvents()
	{
		//if the widget has child widgets
		if(m_widgets.size() > 0) {
			for(int i=0; i<(int)m_events.size(); i++) {
				sf::Event* event = m_events[i];
				switch(event->Type) {
					case sf::Event::MouseButtonPressed:
						{
							WidgetList::reverse_iterator itr = m_widgets.rbegin();
							for(; itr!=m_widgets.rend(); itr++) {
								//you don't get events if dead or hidden
								if(itr->second->m_dead || itr->second->IsHidden()) 
									continue;
								//you don't get events if hidden behind the parent's clip rect
								if(!itr->second->IsCollision(NormalizeClipAreaView())) 
									continue;

								int x = m_events[i]->MouseButton.X;
								int y = m_events[i]->MouseButton.Y;
								sf::Vector2f v = s_gui->GetWindow().ConvertCoords(x,y);
								x = (int)v.x; y = (int)v.y;
								if(itr->second->IsCollision(Rect(x,y,2,2))) {
									SetFocus(itr->second);						
									int X = (int)m_focus->GetPos().x;
									int Y = (int)m_focus->GetPos().y;

									//_StartDrag(x-X,y-Y);
									Drag* drag = itr->second->CreateDrag(event);
									s_gui->RegisterDrag(drag);

									//a child received the event.. so register it
									itr->second->RegisterEvent(event);
	
									//another click happened.. is it within time?
									if(m_doubleClickActivated) {
										if(m_doubleClickDiff < 0) {
											//it was not within time.. double click expired
											m_doubleClickDiff = m_doubleClickTime;
											m_doubleClickActivated = false;
										} else {
											itr->second->OnDoubleClick(event);
											//reset double click
											m_doubleClickActivated = false;
											m_doubleClickDiff = m_doubleClickTime;
										}
									} else { //first click.. activate the double click
										m_doubleClickActivated = true;
										m_doubleClickDiff = m_doubleClickTime;
									}
									break;
								} 
							}
							//if no collisions occurred then it's your event...
							if(itr == m_widgets.rend()) {
								//make current focus lose focus..
								if(m_focus) {
									m_focus->_LoseFocus();
									m_focus = NULL;
								}
								_DispatchEvent(event); //so dispatch it!
							}
						}
						break;
					case sf::Event::MouseButtonReleased:
						if(m_focus) { //if you have focus that means you have child widgets
							sf::Vector2f v;
							v.x = (float)m_events[i]->MouseButton.X;
							v.y = (float)m_events[i]->MouseButton.Y;
							v = s_gui->GetWindow().ConvertCoords((int)v.x,(int)v.y);

							//focus will get the event even if it's not colliding!
							m_focus->RegisterEvent(event);

							//if you're not colliding though.. you will lose focus!
							if(!m_focus->IsCollision(Rect((int)v.x,(int)v.y,1,1))) {
								m_focus->_LoseFocus(false);
								m_focus = NULL;
							}

							//stop drag
							m_drag = false;
						} else {
							//if you are a leaf.. you own the event.. dispatch it!
							if(m_widgets.empty()) {
								_DispatchEvent(event);
							} else {
								sf::Vector2f a((float)event->MouseButton.X,
									(float)event->MouseButton.Y);
								ConvertCoords(a);
							}
						}
						break;
					case sf::Event::MouseMoved:
						{
							sf::Vector2f a((float)event->MouseMove.X,
								(float)event->MouseMove.Y);
							ConvertCoords(a);

							if(m_drag && m_focus) {
								m_focus->SetPos((int)a.x-m_hotSpotX, (int)a.y-m_hotSpotY);
							}
							Widget* temp = NULL;
							bool hadHover = false;
							WidgetList::reverse_iterator it;
							for(it	= m_widgets.rbegin(); it != m_widgets.rend(); it++) {
								if(it->second->IsCollision(Rect((int)a.x,(int)a.y,1,1))) {
									if(!it->second->m_visible) continue;

									//if it's the same widget.. don't count it
									if(m_hoverTarget == it->second) {
										if(m_hoverTarget) m_hoverTarget->RegisterEvent(event);
										break;
									}

									it->second->OnHover();	

									temp = m_hoverTarget;
									m_hoverTarget = it->second;
									m_hoverTarget->RegisterEvent(event);

									break; //the mouse can only hover over 1 widget at a time..
								}
							}
							if(temp) {
								temp->OnHoverLost();
							} else if(it == m_widgets.rend()) {
								if(m_hoverTarget) m_hoverTarget->OnHoverLost();
								m_hoverTarget = NULL;
							}
							OnEvent(event);

						} break;
					default:
						OnEvent(event);

						if(m_focus) {
							m_focus->RegisterEvent(event);
						}
						break;
				}
			}
		} else {	//else you are a leaf .. so you own the event!
			for(uint32 i=0; i<m_events.size();i++) {
				_DispatchEvent(m_events[i]);
			}
		}

		//we solved events..so clear them
		m_events.clear();

		//now update child widgets 
		for(WidgetList::iterator i = m_widgets.begin(); i!= m_widgets.end(); i++){
			i->second->_HandleEvents();
		}	
	}

	void Widget::SetFocus( Widget* widget )
	{
		if(m_focus) {
			m_focus->m_isFocus = false;
			m_focus->_HandleOnFocusLost();
			m_focus = NULL;
		}
		if(!widget) return;

		m_focus = widget;
		widget->m_isFocus = true;
		widget->OnFocus();
	}

	bool Widget::IsFocus() const
	{
		Widget* current = m_parent;
		bool focus = m_isFocus;
		while(current) {
			//you have focus if all your parents have focus as well
			focus = focus && current->m_isFocus;
			current = current->m_parent;
		}
		return focus;
	}

	void Widget::_HandleOnHover( const sf::Vector2f& pos )
	{
		//send the hover event to self and child widgets
		if(!IsHovering())
			OnHover();

		for(WidgetList::iterator it = m_widgets.begin();it!=m_widgets.end();it++) {
			if(it->second->IsCollision(m_rect)) {
				it->second->_HandleOnHover(pos);
				break; //only one widget can have a hover event..
			}
		}
	}

	void Widget::_HandleOnFocusLost()
	{
		//send the focus-lost event to self and child widgets
		OnFocusLost();
		for(WidgetList::iterator it = m_widgets.begin();it!=m_widgets.end();it++) {
			it->second->OnFocusLost();
		}
	}

	void Widget::ReloadTheme()
	{
		//changing themes doesn't matter if the widget has a individual theme
		if(m_individualTheme || !s_gui->GetTheme()) return;

		InitGraphics();

		for(WidgetList::iterator it = m_widgets.begin(); it != m_widgets.end(); it++) {
			it->second->ReloadTheme();
		}

	}

	void Widget::SaveLayout( TiXmlNode* node ) const
	{
		if(!m_allowSave) return;

		//save my properties
		if(m_settings.size()) {
			TiXmlElement* elem = new TiXmlElement("property");
			node->LinkEndChild(elem);
			m_settings.Dump(elem);
		}

		//now save all my child widgets
		for(WidgetList::iterator it = m_widgets.begin(); it != m_widgets.end(); it++) 
		{
			it->second->SaveLayout(node);
		}

	}

	void Widget::ReloadSettings()
	{
		Rect temp;
		bool resize = false;
		bool setpos = false;

		if(m_settings.HasInt32Value("posx")) {
			temp.x = m_settings.GetInt32Value("posx");
			setpos = true;
		}
		if(m_settings.HasInt32Value("posy")) {
			temp.y = m_settings.GetInt32Value("posy");
			setpos = true;
		}
		if(m_settings.HasUint32Value("width")) {
			temp.w = m_settings.GetUint32Value("width");
			resize = true;
		}
		if(m_settings.HasUint32Value("height")) {
			temp.h = m_settings.GetUint32Value("height");
			resize = true;
		}

		if(m_settings.HasStringValue("visibility")) 
		{
			if(m_settings.GetStringValue("visibility") == "on")
				this->Show();
			else this->Hide();
		}
		if(m_settings.HasUint32Value("movable")) {
			if(m_settings.GetUint32Value("movable")) {
				this->SetMovable(true);
			} else this->SetMovable(false);
		}
		if(m_settings.HasUint32Value("vpolicy")) {
			uint32 i = m_settings.GetUint32Value("vpolicy");
			if(i > MaximumExpand) {
				error_log("Widget(\"%s\") contain an invalid size policy! %u",m_name.c_str(),i);
			} else {
				SetVerticalPolicy(SizePolicy(i));
			}
		}
		if(m_settings.HasUint32Value("hpolicy")) {
			uint32 i = m_settings.GetUint32Value("hpolicy");
			if(i > MaximumExpand) {
				error_log("Widget(\"%s\") contain an invalid size policy! %u",m_name.c_str(),i);
			} else {
				SetHorizontalPolicy(SizePolicy(i));
			}
		}

		if(resize) this->Resize(temp.w, temp.h);
		if(setpos) this->SetPos(temp.x, temp.y,true);

		//each widget should init their own kind of graphics
		InitGraphics();
	}

	void Widget::InitGraphics()
	{
		if(m_settings.HasUint32Value("background-color")) {
			m_shape.SetColor(UnsignedToColor(m_settings.GetUint32Value("background-color")));
			m_individualTheme = true;
		} else if(s_gui->GetTheme()){
			m_shape.SetColor(s_gui->GetTheme()->GetColor("background-color"));
		}
	}

	void Widget::_LoseFocus(bool forgetFocus /* = true */)
	{
		OnFocusLost();
		//you're no longer focus
		m_isFocus = false;
		//you no longer remember which child had focus
		if(forgetFocus)
			m_focus = NULL;

		//repeat for all child widgets
		for(WidgetList::iterator it = m_widgets.begin(); it != m_widgets.end(); it++) {
			if(it->second->IsFocus())
				it->second->_LoseFocus();
		}
	}

	//in the form of: "parent.child.leaf"
	std::string Widget::GetWidgetPath() const
	{
		std::stack<Widget*> stack;
		Widget* parent = m_parent;
		while(parent) {
			stack.push(parent);
			parent = parent->GetParent();
		}
		std::string temp;
		while(!stack.empty()) {
			temp = temp + stack.top()->GetName() + '.';
			stack.pop();
		}
		temp += this->GetName();
		
		return temp;
	}

	Settings& Widget::GetSettings() const
	{
		return m_settings;
	}

	void Widget::OnResize(const Rect& oldRect)
	{
		m_mediator.PostEvent(new gui::OnResize(this,oldRect));
	}

	void Widget::OnMove(const Rect& oldRect)
	{
		m_mediator.PostEvent(new gui::OnMove(this,oldRect));
	}

	void Widget::OnShow()
	{
		m_mediator.PostEvent(new gui::OnShow(this));
	}

	void Widget::OnHide()
	{
		m_mediator.PostEvent(new gui::OnHide(this));
	}

	void Widget::OnDestroy()
	{
		//m_mediator.PostEvent(new Event::OnDestroy(this));
	}

	void Widget::OnHover()
	{
		m_hovering = true;
		m_mediator.PostEvent(new gui::OnHover(this));
	}

	void Widget::OnHoverLost()
	{
// 		glEnable(GL_SCISSOR_TEST);
// 		glDisable(GL_SCISSOR_TEST);
		m_hovering = false;
		m_mediator.PostEvent(new gui::OnHoverLost(this));
	}

	void Widget::OnFocus()
	{
		m_mediator.PostEvent(new gui::OnFocus((gui::Widget*)this));
	}

	void Widget::OnFocusLost()
	{
		m_mediator.PostEvent(new gui::OnFocusLost(this));
	}

	bool Widget::IsHovering() const
	{
		return m_hovering;
	}

	bool Widget::IsCollision( const Rect& rect ) const
	{
		return m_visible ? gui::IsCollision(m_rect,rect) : false;
	}

	bool Widget::IsCollision( const Rect& rect, sf::Vector2f& normal ) const
	{
		return m_visible ? gui::IsCollision(m_rect,rect, normal) : false;
	}

	void Widget::SaveUI( TiXmlNode* node ) const
	{
		if(!node || !m_allowSave) return;

		TiXmlElement* e = new TiXmlElement("widget");
		TiXmlComment* c = NULL;

		e->SetAttribute("name", GetName().c_str());
		e->SetAttribute("type", (int)GetType());
		node->LinkEndChild(e);

		if(m_mediator.IsActive()) {
			c = new TiXmlComment("Internal listeners. Usable inside specific widgets");
			e->LinkEndChild(c);

			//save listeners
			m_mediator.SaveConnections(e);
		}

		//save my properties
		if(m_settings.size()) {
			c = new TiXmlComment("widget properties:");
			e->LinkEndChild(c);

			TiXmlElement* elem = new TiXmlElement("property");
			m_settings.Dump(elem);
			e->LinkEndChild(elem);
		}

		if(m_widgets.size()) {
			c = new TiXmlComment("child widgets:");
			e->LinkEndChild(c);
		}

		//now save all my child widgets
		for(WidgetList::iterator it = m_widgets.begin(); it != m_widgets.end(); it++) 
		{
			it->second->SaveUI(e);
		}

	}

	Mediator& Widget::GetMediator() const
	{
		return m_mediator;
	}

	bool Widget::IsSolid() const
	{
		return m_solid;
	}

	void Widget::SetSolid( bool flag )
	{
		m_solid = flag;
	}

	bool Widget::CanSave() const
	{
		return m_allowSave;
	}

	void Widget::AllowSave( bool flag )
	{
		m_allowSave = flag;
	}

	void Widget::ResolveChildCollisions()
	{
		//resolve solid collisions between your child widgets
		sf::Vector2f normal;
		for(WidgetList::iterator it = m_widgets.begin(); 
			it != m_widgets.end(); it++) 
		{
			for(WidgetList::iterator i = m_widgets.begin();
				i != m_widgets.end(); i++) 
			{
				if(!i->second->IsSolid() || it == i) continue;

				if(it->second->IsCollision(i->second->GetRect(),normal)) {
					int xpos = it->second->GetRect().x + (int)normal.x;
					int ypos = it->second->GetRect().y + (int)normal.y;
					it->second->SetPos(xpos, ypos,true);
				}
			}
		}
	}

	sf::RenderWindow* Widget::GetWindow() const
	{
		return &s_gui->GetWindow();
	}

	Drag::DropFlags Widget::GetDropFlags() const
	{
		return m_dropFlags;
	}

	bool Widget::AcceptsDrop( Drag* drag ) const
	{
		if(!drag) return false;
		if(drag->GetType() != Drag::Widget) return false;

		return true;
	}

	Drag* Widget::CreateDrag( sf::Event* event )
	{
		//this is the basic drag for the basic widget, other widgets may 
		//have other types of drag, (text drag, image drag etc)
		return new Drag(this,event);
	}

	bool Widget::HandleDragMove( Drag* drag )
	{
		if(!drag) return false;
		if(drag->GetStatus() != Drag::Running) return false;

		Widget* target = drag->GetTarget();
		
		//basic widget specific child-movement policy, 
		//other widget might move child widgets differently
		return target->SetPosFromDrag(drag);
	}

	bool Widget::HandleDragDrop( Drag* drag )
	{
		//basic widget specific receive drop policy
		//only accept drops of type Widget
		if(!drag) return false;

		//this should never happen, but check anyways
		if(drag->GetType() != Drag::Widget) return false;

		//the default receive drop policy is to add the widget to this
		AddWidgetForced(drag->GetTarget());

		return true;
	}

	bool Widget::HandleDragDraw( Drag* drag )
	{
		//basic widget drag-drawing policy, by default it draws nothing 
		//because it just moves the widget, so that'll act as the draw
		
		sf::String pos;
		std::stringstream s;
		s << "x: " << drag->GetCurrentPos().x << " |y: " << drag->GetCurrentPos().y;
		pos.SetText(s.str());
		pos.SetColor(sf::Color(255,0,0));
		pos.SetPosition(drag->GetCurrentPos());
		s_gui->GetWindow().Draw(pos);
		return true;
	}

	bool Widget::HandleDragStop( Drag* drag )
	{
		//basic widget drag-stop policy, other might do something different
		if(!drag) return false;
		if(drag->GetStatus() != Drag::Finished) return false;

		Widget* target = drag->GetTarget();

		//if the drag failed, move the target back to its original position
		if(drag->GetDropStatus() == Drag::Failed) {
			target->SetPos(drag->GetStartPos(),true); //forced move
		} else if(drag->GetDropStatus() == Drag::Succesful) {
			//if it's been moved outside the widget
			if(drag->GetCurrentFocus() != this) {
				//if it's been moved on the gui
				if(!drag->GetCurrentFocus()) {
					//order matters, because when you add the widget the m_id changes!
					RemoveWidget(target);	  //remove it first
					s_gui->AddWidget(target); //then add it to someone else
				} else {
					//it's inside another widget.. 
					Widget* parent = drag->GetCurrentFocus();
					//parent->AddWidget(target);

					//m_id of target will likely change if the drop is successful and the parent changes
					uint32 old_id = drag->GetTarget()->GetId();
					if(!parent->HandleDragDrop(drag)) {
						//move to the original drag position.. since drag failed
						drag->GetTarget()->SetPos(drag->GetStartPos(),true);
					} else {

						//remember the new id received
						uint32 new_id = drag->GetTarget()->GetId();

						//need this to be able to remove the widget from management
						drag->GetTarget()->SetId(old_id);
						RemoveWidget(target);

						//restore the new id received
						drag->GetTarget()->SetId(new_id);

					}
					return true;
				}
			}

			//update position
			target->SetPos(drag->GetCurrentPos(),drag->GetForcedMove());
		}
		return true;
	}

	Widget* Widget::GetWidgetAt( int x, int y ) const
	{
		Rect rect = Rect(x,y,1,1);
		for(WidgetList::const_iterator it = m_widgets.begin(); 
			it != m_widgets.end(); it++) 
		{
			if(it->second->IsCollision(rect)) {
				return it->second;
			}
		}
		return NULL;	
	}

	Widget* Widget::GetWidgetAt( const sf::Vector2f& pos ) const
	{
		return GetWidgetAt((int)pos.x, (int)pos.y);
	}

	//in this order: leaf->child->parent
	std::vector<Widget*> Widget::GetWidgetStackPath() const
	{
		std::vector<Widget*> v;
		v.push_back(const_cast<Widget*>(this));

		Widget* parent = m_parent;
		while(parent) {
			v.push_back(parent);
			parent = parent->GetParent();
		}
		return v;
	}

	bool Widget::SetPosFromDrag( Drag* drag )
	{
		Drag::DropFlags temp = m_dropFlags;
			m_dropFlags = drag->GetDragFlags();
			SetPos(drag->GetCurrentPos(), drag->GetForcedMove());
			
			//create the drag event.. you shouldn't really create a new drag event every time you move the mouse :|
			m_mediator.PostEvent(new gui::OnDrag(this,drag));

		m_dropFlags = temp;

		return true;
	}

	Widget* Widget::QueryWidget( const std::string& path ) const
	{
		std::vector<std::string> temp;
		ExtractPath(path,temp);

		if(!temp.size()) return NULL;

		Widget* cur = NULL;
		for(uint32 i=0; i<temp.size(); i++) {
			cur = FindChildByName(temp[i]);
			if(!cur) {
				error_log("Unable to complete query \"%s\". Widget \"%s\" not found!", path.c_str(),temp[i]);
				return NULL;
			}
		}
		return cur;
	}


	bool Widget::IsDead() const
	{
		return m_dead;
	}

	void Widget::Kill()
	{
		m_dead = true;

		//kill all contained widgets!
		for(WidgetList::iterator it = m_widgets.begin(); it != m_widgets.end(); it++) {
			it->second->Kill();
		}
	}

	bool Widget::IsMovable() const
	{
		return m_movable;
	}

	void Widget::AddWidgetForced( Widget* child )
	{
		uint32 attempts = 1;
		std::string name = child->GetName();
		while(true) {
			if(!AddWidget(child)) {
				std::stringstream s;
				s << name;

				s << "_" << attempts;
				child->SetName(s.str());
				attempts++;
			} else return;
		}
	}

	void Widget::StartClipping() const
	{
		if(!s_gui) return;
		glEnable(GL_SCISSOR_TEST);
		glScissor(m_clipRect.x, m_clipRect.y, m_clipRect.w, m_clipRect.h); 
	}

	void Widget::StopClipping() const
	{
		glDisable(GL_SCISSOR_TEST);
	}

	bool Widget::IsVisible() const
	{
		return m_visible;
	}

	bool Widget::IsHidden() const
	{
		return !m_visible;
	}

	void Widget::UpdateClipArea()
	{
// 		if(!s_gui) return;
// 		m_clipRect = m_rect;
// 		sf::Vector2f pos = m_rect.GetPos();
// 		ConvertCoords(pos);
// 		m_clipRect.x = (int)pos.x;
// 		m_clipRect.y = s_gui->GetWindow().GetHeight() - (int)pos.y - m_rect.h;
		ResizeClipArea(s_gui->GetOldWidth(),s_gui->GetOldHeight());
	}

	//returns the visible rect --view based!)
	gui::Rect Widget::NormalizeClipAreaView() const
	{
		if(!s_gui || !m_clipRect) return Rect();

		Rect temp = m_clipRect;

		double height = s_gui->GetWindow().GetView().GetHalfSize().y*2;
		double width = s_gui->GetWindow().GetView().GetHalfSize().x*2;

		double yprc = height/s_gui->GetWindow().GetHeight();
		double xprc = width/s_gui->GetWindow().GetWidth();

		temp.y = uint32(m_rect.y);	//slight hack?
		temp.x = uint32(m_clipRect.x * xprc);
		temp.w = uint32(m_clipRect.w * xprc);
		temp.h = uint32(m_clipRect.h * yprc);

		return temp;
	}

	//returns the visible rect as seen on screen(coords are screen..not view based!)
	gui::Rect Widget::NormalizeClipArea() const
	{
		if(!s_gui || !m_clipRect) return Rect();

		Rect temp = m_clipRect;

		double height = s_gui->GetWindow().GetView().GetHalfSize().y*2;
		double yprc = s_gui->GetWindow().GetHeight()/height;
		temp.y = uint32(m_rect.y * yprc);

		return temp;
	} 

	void Widget::ResizeClipArea( uint32 newWidth, uint32 newHeight )
	{
// 		m_clipRect.w *= newWidth;
// 		m_clipRect.h *= newHeight;
		double width = s_gui->GetWindow().GetView().GetHalfSize().x*2;
		double height = s_gui->GetWindow().GetView().GetHalfSize().y*2;

		double xprc = newWidth/width;
		double yprc = newHeight/height;
		double wprc = newWidth/width;
		double hprc = newHeight/height;

		m_clipRect = m_rect;

		m_clipRect.x = int32(m_clipRect.x * xprc);
		m_clipRect.y = int32(m_clipRect.y * yprc);
		m_clipRect.w = uint32(m_clipRect.w * wprc);
		m_clipRect.h = uint32(m_clipRect.h * hprc);
		
		m_clipRect.y = newHeight - m_clipRect.y - m_clipRect.h;

	}

	gui::Widget::SizePolicy Widget::GetVerticalPolicy() const
	{
		return m_verticalPolicy;
	}

	gui::Widget::SizePolicy Widget::GetHorizontalPolicy() const
	{
		return m_horizontalPolicy;
	}

	void Widget::SetSizeHint( const sf::Vector2i& size )
	{
		m_sizeHint = size;
	}

	const sf::Vector2i& Widget::GetSizeHint() const
	{
		return m_sizeHint;
	}

	//checks whether a widget contains the other.. self included
	bool Widget::ContainsWidget( Widget* widget )
	{
		if(this == widget) return true;

		for(WidgetList::iterator it = m_widgets.begin(); 
			it != m_widgets.end(); it++) 
		{
			if(widget == it->second) {	//if your child is the wanted one.. you contain it!
				return true;
			} else if(it->second->ContainsWidget(widget)) { //else if your child contains it.. you contain it as well!
				return true;
			}
		}

		return false; //you don't have it...
	}

	void Widget::SetLoading( bool val )
	{
		m_loading = val;
	}

	bool Widget::IsLoading() const
	{
		return m_loading;
	}

	void Widget::SetVerticalPolicy( SizePolicy policy )
	{
		m_settings.SetUint32Value("vpolicy",policy);
		m_verticalPolicy = policy;
	}

	void Widget::SetHorizontalPolicy( SizePolicy policy )
	{
		m_settings.SetUint32Value("hpolicy", policy);
		m_horizontalPolicy = policy;
	}

	void Widget::OnDoubleClick( sf::Event* event )
	{
		m_mediator.PostEvent(new gui::OnDoubleClick(this));
	}
}
