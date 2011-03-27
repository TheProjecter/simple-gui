#pragma once

#include "Defines.hpp"

namespace gui
{
	class Widget;

	class Drag 
	{
	public:

		enum DragStatus {
			None,
			Running,
			Finished
		};

		enum DropStatus {
			Failed,
			Succesful
		};

		//allowed places to drop a widget
		enum DropFlags {
			ParentOnly,
			WidgetOnly,
			Anywhere
		};

		enum DragType {
			Widget,
			Text,
			Image
		};

		Drag(gui::Widget* target, sf::Event* event);
		virtual ~Drag();

		void Update(float diff);

		const sf::Vector2f& GetStartPos() const;
		const sf::Vector2f& GetCurrentPos() const;
		void SetPos(const sf::Vector2f& pos);

		uint32 GetDropStatus() const;
		uint32 GetStatus() const;
		uint32 GetDragType() const;

		void StopDrag();
		bool IsRunning() const;
		bool IsFinished() const;

		void SetForcedMove(bool flag);
		bool GetForcedMove() const;

		gui::Widget* GetTarget() const;
		gui::Widget* GetTargetParent() const;
		gui::Widget* GetCurrentFocus() const;
	protected:
		sf::Vector2f m_startPos;	//the widget position where the dragging started
		sf::Vector2f m_curPos;		//the current position of where the widget will land
		sf::Vector2f m_hotSpot;		//the place withing the widget where the drag started

		uint32 m_dropStatus;		//whether the drop was successful
		std::string m_dropErrors;	//stores the results when a drop fails
		uint32 m_status;			//current status of the drag(NONE,RUNNING,FINISHED)
		uint32 m_type;				//the drag type (Widget, Text, Image, etc?)

		uint32 m_curDragDist;		//the current drag distance, will stop updating when in RUNNING status
		uint32 m_minDragDist;		//the min amount of distance it takes for drag to start
		
		gui::Widget* m_target;		//the widget being dragged
		gui::Widget* m_targetParent;//the target's parent, if any.. do I need it ?
		gui::Widget* m_focusTarget;	//the *appropriate* widget over which the mouse hovers

		bool m_forceMove;			//forces any widget to move irregardless of flags
	private:
	};
}