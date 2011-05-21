#pragma once

#include "Defines.hpp"
#include "Theme.hpp"
#include <map>
#include "GuiMgrParser.hpp"
#include "Mediator.hpp"
#include "AbstractFactory.hpp"

class TiXmlNode;

namespace gui {

	class Widget;
	class Drag;

	class GuiManager 
	{
	public:
		GuiManager(sf::RenderWindow& window);
		~GuiManager();

		void Initialize(sf::RenderWindow * renderTarget);

		void SetTheme(Theme* theme);
		Theme* GetTheme() const;

		void SetWindow(sf::RenderWindow* window);
		sf::RenderWindow& GetWindow() const;
		void RegisterEvent(sf::Event& event);

		bool AddWidget(Widget* widget);
		void DeleteWidget(const std::string& name);
		void DeleteWidget(Widget* widget);

		void Update(float diff);
		const Mediator& GetMediator() const { return m_mediator; }

		std::vector<Widget*> GetWidgetsByType(WidgetType type) const;
		Widget* GetWidgetByName(const std::string& name) const;
		Widget* GetWidgetAt(const sf::Vector2f& pos) const;
		Widget* GetWidgetAt(int x, int y) const;

		void SaveLayout(const char* filename);
		bool LoadLayout(const char* filename);

		void SaveUI(const char* filename);
		void LoadUI(const char* filename);

		void RegisterFactory(AbstractFactory* userFactory);
		void RegisterDrag(Drag* drag);

		//if the query fails, the stack will be empty!
		std::stack<Widget*> QueryWidgetPath(const std::string& path) const;
		Widget* QueryWidget(const std::string& path) const;
		sf::Vector2f ConvertCoords(int x, int y);

		gui::uint32 GetOldWidth() const;
		gui::uint32 GetOldHeight() const;
	private:
		friend class GuiMgrParser;
		typedef std::map<uint32, Widget*> WidgetList;
		std::vector<Widget*> m_freeWidgets;	//holds the guids of widgets that will be freed
		Theme* m_theme;						//the current theme used by widgets
		WidgetList m_widgets;				//holds the parent widgets
		uint32 index;						//used to keep track of focusing levels
		std::vector<sf::Event> m_events;	//events queue
		Widget* m_focus;					//holds the currently focused widget if any
		Widget* m_hoverTarget;
		sf::RenderWindow& m_window;			//pointer to the window we're working on
		uint32 m_hotSpotX, m_hotSpotY;		//used for dragging			
		bool m_drag;						//flag for determining whether drag is happening
		std::vector<AbstractFactory*> m_factories;
		GuiMgrParser m_parser;
		mutable Mediator m_mediator;
		Drag* m_curDrag;

		//used when resizing
		uint32 m_oldWidth;
		uint32 m_oldHeight;

		void SetHasFocus(WidgetList::reverse_iterator& i);
		void StartDrag(Widget* widget, sf::Event* event);	//deprecated?
		void MoveDrag(sf::Event* event);
		void StopDrag(int x, int y);				

		Widget* GetLastWidgetAt(int x, int y, Widget* skip = NULL) const;
		void UpdateDragFocus(int x, int y);
		void ClearWidgets();
		void FreeWidgets();
		void _HandleEvents();


	};

}