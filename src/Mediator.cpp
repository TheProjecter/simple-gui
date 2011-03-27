#include "../include/gui/Mediator.hpp"
#include "../include/gui/Debug.hpp"
#include <tinyxml.h>
#include "../include/gui/Widget.hpp"

namespace gui 
{

	Dispatcher& Mediator::GetDispatcher() const
	{
		return m_dispatcher;
	}


	Mediator::Mediator()
	{
		m_freeEvent = NULL;	
		m_listeners["default"] = Listener();
		m_currentListener = &m_listeners["default"];
	}

	bool Mediator::SwitchListener( const std::string& listener /* = "default" */) const
	{
		m_currentListener = &m_listeners[listener];
		return true;
	}

	Event* Mediator::GetEvent() const
	{
		if(!m_currentListener) return NULL;
		
		//free memory for events when count reaches 0
		if(m_freeEvent) {
			delete m_freeEvent;
			m_freeEvent = NULL;
		}

		Event* temp = m_currentListener->pop();
		if(!temp) return NULL;

		if(temp->m_refCount == 0)
			m_freeEvent = temp;
		
		return temp;
	}

	Listener* Mediator::GetListener( const std::string& listener /*= "default"*/ ) const
	{
		return &m_listeners[listener];
	}

	void Mediator::PostEvent( Event* event )
	{
		m_dispatcher.DispatchEvent(event);
	}

	Mediator::~Mediator()
	{
		for(ListnerList::iterator it = m_listeners.begin(); it != m_listeners.end(); it++) {
			//free events from all listeners
			SwitchListener(it->first);

			while(GetEvent()) {
				//consume events.. so they get freed
			}
		}
		ClearConnections();
	}

	bool Mediator::IsActive() const
	{
		return (m_connections.size() != 0);
	}

	bool Mediator::SetDefaultListener() const
	{
		return SwitchListener("default");
	}

	//should receive a <widget> element node/ or a document node
	void Mediator::SaveConnections( TiXmlNode* node ) const
	{
		if(!node) return;

		for(ConnectionList::const_iterator it = m_connections.begin(); 
			it != m_connections.end(); it++)
		{
			const std::string& listener_name = it->first;
			TiXmlElement* elem = new TiXmlElement("listener");
			elem->SetAttribute("name",listener_name.c_str());

			for(uint32 i=0; i<it->second.size(); i++)
			{
				std::vector<ListenerEventPair>& v = it->second[i]->widgetPaths;
				if(it->second[i]->save == false) continue;

				for(uint32 j=0; j<v.size(); j++) 
				{
					std::string& widgetPath = v[j].first;
					uint32 eventType = v[j].second;

					TiXmlElement* e = new TiXmlElement("widget");
					e->SetAttribute("name",widgetPath.c_str());
					e->SetAttribute("event",(int)eventType);

					elem->LinkEndChild(e);
				}		
			}
			node->LinkEndChild(elem);
		}
	}		

	Listener* Mediator::GetCurrentListener() const
	{
		return m_currentListener;
	}

	void Mediator::Connect( Widget* widget, const std::string& my_listener, 
							uint32 eventType, 
							bool saveConnection /*= true*/ )
	{
		if(!widget) return;

		Mediator& with = widget->GetMediator();
		Listener* myListener = GetListener(my_listener);
		
		if(!myListener) return;

		with.GetDispatcher().RegisterListener(eventType,myListener);

		ConnectionInfo* c = new ConnectionInfo(saveConnection);
		c->widgetPaths.push_back(ListenerEventPair(widget->GetWidgetPath(),eventType));

		m_connections[my_listener].push_back(c);
	}

	void Mediator::ClearConnections()
	{
		for(ConnectionList::iterator it = m_connections.begin(); it != m_connections.end(); it++) {
			for(uint32 i=0; i<it->second.size(); i++) {
				delete it->second[i];
			}
		}
		m_connections.clear();
	}

	bool Dispatcher::CaresAbout( uint32 eventType )
	{
		return ((m_listeners.find(eventType) != m_listeners.end()) ||	//particular interests...
				(m_listeners[uint32(-1)].size()));						//general purpose.. cares about any event!
	}

	void Dispatcher::DispatchEvent( Event* event )
	{
		if(!event) return;

		std::map<uint32,std::vector<Listener*> >::iterator it = m_listeners.find(event->GetType());
		
		//if nobody cares about this event.. free the memory
		if(it == m_listeners.end()) {
			const std::vector<Listener*>& generalListeners = m_listeners[uint32(-1)];

			if(!generalListeners.size()) {
				delete event;
				return;
			}
			//if some cares about any event.. send it to it
			for(uint32 i=0; i<generalListeners.size(); i++) {
				generalListeners[i]->push(event);
			}
			
		} else {
			//dispatch event to all interested listeners
			for(uint32 i=0; i<it->second.size(); i++) {
				it->second[i]->push(event);
			}
		}
	}

	void Dispatcher::RegisterListener( Listener* listener )
	{
		uint32 i = (uint32)-1; //max value
		m_listeners[i].push_back(listener);
	}

	void Dispatcher::RegisterListener( uint32 eventType, Listener* listener )
	{
		m_listeners[eventType].push_back(listener);
	}

	void Listener::push(Event* e)
	{
		//clear all other events 
		if(m_events.size() > m_maxCount) {
			while(m_events.size()) {
				Event* e = pop();

				//if no one else cares about this event.. go ahead and free it
				if(e->m_refCount == 0) {
					delete e;
				}
			}
		}
		m_events.push(e);
		e->m_refCount++;
	}
	Event* Listener::pop()
	{
		if(!m_events.size()) return NULL;

		Event* e = m_events.front();
		if(e->m_refCount > 0) {
			e->m_refCount--;
		} else {
			error_log("Tried to decrease refCount but it is 0. It should get freed!");
		}
		m_events.pop();
		return e;
	}

	Listener::Listener() : m_maxCount(50) //max allowed events
	{

	}


	ConnectionInfo::ConnectionInfo( bool saveConnection ) : save(saveConnection)
	{

	}
}
