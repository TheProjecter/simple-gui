#pragma once

#include "Defines.hpp"
#include <map>
#include "Event.hpp"
#include <queue>

class TiXmlNode;

namespace gui
{

	class Widget;

	struct Listener {
	public:
		Listener();
		void push(Event*);
		Event* pop();
		void clear();

	private:
		uint32 m_maxCount;
		std::queue<Event*> m_events;
	};

	typedef std::map<std::string, Listener > ListnerList;
	typedef std::pair<std::string,uint32> ListenerEventPair;

	class Dispatcher
	{
	public:
		Dispatcher() {}
		bool CaresAbout(uint32 eventType);
		void RegisterListener(uint32 eventType, Listener* listener);
		void RegisterListener(Listener* listener);
		bool UnRegisterListner(uint32 eventType, Listener* listener);
		void DispatchEvent(Event* event);
		void ClearListeners();
	private:
		std::map<uint32, std::vector<Listener*> > m_listeners;
	};

	class Mediator;

	//TODO: i don't need the saveConnection anymore.. i can just do a if(!saveconnection) return;
	struct ConnectionInfo {
		ConnectionInfo(bool saveConnection);
		bool save;
		std::vector<ListenerEventPair > widgetPaths;	//the path to the widget registered
	};

	class GuiManager;

	class Mediator
	{
	public:
		friend class GuiManager;
		Mediator();
		~Mediator();
		Dispatcher& GetDispatcher() const;

		Event* GetEvent() const;
		void PostEvent(Event* event);
		void ConsumeEvents();

		bool SetDefaultListener() const;
		bool SwitchListener(const std::string& listener = "default") const;

		Listener* GetListener(const std::string& listener = "default") const;
		Listener* GetCurrentListener() const;

		//whether it has connections..(listeners ..active)
		bool IsActive() const;

		//connects the current mediator to the widget passed as parameter
		//to the specific eventType
		bool Connect(Widget* with, const std::string& my_listener, 
					 uint32 eventType, bool saveConnection = true);

		//connects the current mediator to the widget found by executing
		//the query. The widget must be registered in the gui mgr to work!
		bool Connect(const std::string& path, const std::string& my_listner,
					 uint32 eventType, bool saveConnection = true);

		//connects the current mediator to the widget found by executing the
		//query starting from the current widget passed by parameter
		bool Connect(Widget* current, const std::string& path, 
					const std::string& my_listener, uint32 eventType,
					bool saveConnection = true);

		void SaveConnections(TiXmlNode* node) const;
		void ClearConnections();
		void SetCurrentPath(const std::string& path);
	private:

		mutable Listener* m_currentListener;
		mutable Dispatcher m_dispatcher;
		mutable ListnerList m_listeners;
		mutable Event* m_freeEvent;

		std::string m_currentPath;				//widget path
		static GuiManager* s_currentGui;		//current gui

		//used to keep track of connection created (for saving ui)
		
		typedef std::map<std::string, std::vector<ConnectionInfo*> > ConnectionList;
		ConnectionList m_connections;

	};
}