#pragma once

#include <string>
#include "Defines.hpp"
#include "Settings.hpp"
#include "Theme.hpp"
#include "Mediator.hpp"
#include "Drag.hpp"

class TiXmlNode;

namespace gui {

	class Widget
	{
	public:
		friend class GuiManager;
		friend class GuiMgrParser;
		typedef std::map<uint32, Widget*> WidgetList;	

		Widget();
		Widget(const std::string& name);
		virtual ~Widget();

		virtual bool AddWidget(Widget* child);
		virtual void DeleteWidget(const std::string& widgetName);

		uint32 GetType() const;
		void SetType(uint32 type);

		bool CanSave() const;
		void AllowSave(bool flag);

		bool IsSolid() const;
		void SetSolid(bool flag);

		Widget* FindChildByName(const std::string& name);
		std::string GetWidgetPath() const;
		WidgetList& GetWidgetList() const;
		Settings& GetSettings() const;

		virtual void SetParent(Widget* parent);
		Widget* GetParent() const;
		bool HasWidgets() const;
		bool CanDrag(int x, int y) const;

		void SetBackgroundColor(sf::Color color);
		virtual void ReloadSettings();
		Mediator& GetMediator() const;

		virtual void Resize(int w, int h);
		virtual void SetPos(int x, int y, bool forceMove = false);
		virtual void SetPos(const sf::Vector2f& pos, bool forceMove = false);
		void Move(int x, int y );

		uint8 GetTransparency() const;
		void SetTransparency(uint8 val);

		const Rect& GetRect() const;
		sf::Vector2f GetPos() const;
		sf::Vector2f GetSize() const;

		void SetName(const std::string& newName);
		const std::string& GetName() const;

		uint32 GetId() const;
		void SetId(uint32 id);

		void RegisterEvent(sf::Event* event);

		/* Attributes Modifiers */
		void SetMovable(bool flag);
		void Show();
		void Hide();
		bool Clicked() const;
		void SetClicked(bool flag);
		void ShowBackground();
		void HideBackground();

		void HideChildren();
		void ShowChildren();

		virtual bool IsCollision(const Rect& rect) const;
		virtual bool IsCollision(const Rect& rect, sf::Vector2f& normal) const;
		bool IsFocus() const;
		bool IsHovering() const;

		sf::RenderWindow* GetWindow() const;

		Drag::DropFlags GetDropFlags() const;
		virtual bool AcceptsDrop(Drag* drag) const;

	protected:
		/* Attributes */
		uint32 m_type;						//the type of the widget		
		mutable WidgetList m_widgets;		//holds the child widgets
		Widget* m_parent;					//the parent widget if any
		Widget* m_focus;					//the child widget in focus if any
		Widget* m_hoverTarget;				//the current child widget that has hover
		std::vector<sf::Event*> m_events;	//event queue
		sf::Shape m_shape;					//the background image
		sf::Sprite* m_sprite;				//used to draw images.. only use it when needed else let it NULL
		uint32 m_id;						//used by parents to find them in the widget list
		uint32 index;						//internally used to keep track of focusing levels
		std::string m_name;					//name of the widget
		bool m_needUpdate;					//currently unused?
		bool m_isFocus;						//whether the widget has focus either globally or in another widget
		bool m_clicked;						//was the widget clicked ?
		bool m_released;					//was the click released ?
		Rect m_rect;						//the position/size of the widget
		bool m_visible;						//widget visibility
		bool m_mainVisible;					//background image visibility
		bool m_movable;						//is the widget movable ?
		bool m_solid;						//can other widgets pass through this?
		bool m_resizable;					//whether the widget can be resized
		bool m_hovering;					//whether the mouse is currently hovering over the widget
		bool m_allowSave;					//whether the widget will be saved in the ui/layout
		uint8 m_transparency;				//between 0-255
		bool m_individualTheme;				//whether the current theme applies to this widget or not
		mutable Settings m_settings;		//stores changed settings,size,pos,colors etc to be saved/loaded
		mutable Mediator m_mediator;		//handles communication between other widgets and/or gui
		Drag::DropFlags m_dropFlags;		//specifies the drop policy(where the widget may be dropped)
		
		/* Static member data */
		static GuiManager* s_gui;			//pointer to the current gui


		static void ConvertCoords(sf::Vector2f& coords);

		/* Widget specific events */
		void OnResize();
		void OnMove();
		void OnShow();
		void OnHide();
		void OnDestroy();
		void OnHover();
		void OnHoverLost();
		void OnFocus();
		void OnFocusLost();

		//event handling functions -- overload these in derivate widgets
		virtual void OnEvent(sf::Event* event);
		virtual void OnClickPressed(sf::Event* event);
		virtual void OnClickReleased(sf::Event* event);
		virtual void OnKeyPressed(sf::Event* event);
		virtual void OnKeyReleased(sf::Event* event);
		virtual void OnOtherEvents(sf::Event* event);
		virtual void OnTextEntered(sf::Event* event);

		//child handling functions -- overload these
		virtual void OnChildEvent(Widget* child, sf::Event* event);

		virtual void OnChildClickPressed(Widget* child,sf::Event* event);
		virtual void OnChildClickReleased(Widget* child,sf::Event* event);
		virtual void OnChildKeyPressed(Widget* child, sf::Event* event);
		virtual void OnChildKeyReleased(Widget* child, sf::Event* event);
		virtual void OnChildTextEntered(Widget* child, sf::Event* event);
		virtual void OnChildOtherEvents(Widget* child, sf::Event* event);

		void _HandleEvents();
		virtual void Update(float diff);
		virtual void Draw() const;
		void Draw(const sf::Image* image);
		void SetFocus(Widget* widget);
		virtual void ReloadTheme();
		virtual void InitGraphics();


		virtual Drag* CreateDrag(sf::Event* event);

		virtual void HandleDragMove(Drag* drag);	//will be called when the drag moves
		virtual void HandleDragDraw(Drag* drag);	//will be called when the drag needs to be drawn
		virtual void HandleDragStop(Drag* drag);	//will be called when the drag ended

	private:
		bool m_drag;
		int m_hotSpotX, m_hotSpotY;
		void _DispatchEvent(sf::Event* event);
		void _StartDrag(int x, int y);
		void _LoseFocus(bool forgetFocus = true);
		void _HandleOnHover(const sf::Vector2f& pos);
		void _HandleOnFocusLost();
		void SaveLayout(TiXmlNode* node) const;
		void SaveUI(TiXmlNode* node) const;
		void ResolveChildCollisions();
		
	};


}

