#include "../include/gui/Drag.hpp"
#include "../include/gui/Defines.hpp"
#include "../include/gui/Widget.hpp"

namespace gui
{
	const sf::Vector2f& Drag::GetStartPos() const
	{
		return m_curPos;
	}

	const sf::Vector2f& Drag::GetCurrentPos() const
	{
		return m_curPos;
	}

	void Drag::SetPos( const sf::Vector2f& pos )
	{
		if(m_status == None) {
			float dist = ComputeDistance(pos,m_curPos);
			if((uint32)dist > m_minDragDist ) 
				m_status = Running;
		} else if(m_status == Running) {
			m_curPos.x = pos.x - m_hotSpot.x;
			m_curPos.y = pos.y - m_hotSpot.y;
		}
	}

	Drag::Drag( gui::Widget* target, sf::Event* event ):
		  m_target(target), m_forceMove(false), m_type(Drag::Widget)
	{
		if(m_target) {
			if(event) {
				//get the current position of the widget
				m_startPos = m_target->GetRect().GetPos();
				m_curPos = m_startPos;

				//get the position of the click event
				sf::Vector2f temp = m_target->GetWindow()->ConvertCoords(event->MouseButton.X, event->MouseButton.Y);

				//get the place within the widget where the click happened(aka hotspot)
				m_hotSpot.x = temp.x - m_startPos.x;
				m_hotSpot.y = temp.y - m_startPos.y;

				m_targetParent = m_target->GetParent();
			} else {
				error_log("Created drag with NULL event! Possible Crash ?");
			}
		} else {
			error_log("Created drag with NULL target! Possible Crash ?");
		}
	}

	Drag::~Drag()
	{
	}

	uint32 Drag::GetStatus() const
	{
		return m_status;
	}

	bool Drag::IsRunning() const
	{
		return m_status == Running;
	}

	bool Drag::IsFinished() const
	{
		return m_status == Finished;
	}

	void Drag::Update( float diff )
	{
	}

	Widget* Drag::GetTarget() const
	{
		return m_target;
	}

	Widget* Drag::GetTargetParent() const
	{
		return m_targetParent;
	}

	Widget* Drag::GetCurrentFocus() const
	{
		return m_focusTarget;
	}

	uint32 Drag::GetDropStatus() const
	{
		return m_dropStatus;
	}

	void Drag::StopDrag()
	{
		if(m_status == Running)
			m_status = Finished;

		if(!m_focusTarget) {
			//if the current target can't be dropped directly to the gui, the drop failed!
			if(m_target->GetDropFlags() != Anywhere) {
				m_dropStatus = Failed;
			}
		} else {
			if(!m_target->AcceptsDrop(this)) m_dropStatus = Succesful;
			else m_dropStatus = Failed;
		}
	}

	void Drag::SetForcedMove( bool flag )
	{
		m_forceMove = flag;
	}

	bool Drag::GetForcedMove() const
	{
		return m_forceMove;
	}

	gui::uint32 Drag::GetDragType() const
	{
		return m_type;
	}

}
