#include "Win32Input.h"

namespace Happic { namespace Core {

	Win32Input::Win32Input(const HWND & handle)
	{
		m_handle = handle;
		m_isCursorVisible = true;
	}

	Math::Vector2f Win32Input::GetCursorPosition() const
	{
		POINT cursorPos;
		GetCursorPos(&cursorPos);
		ScreenToClient(m_handle, &cursorPos);

		return Math::Vector2f(cursorPos.x, cursorPos.y);
	}

	void Win32Input::SetCursorPosition(const Math::Vector2f & pos) const
	{
		POINT point;
		point.x = pos.GetX();
		point.y = pos.GetY();

		ClientToScreen(m_handle, &point);
		SetCursorPos(point.x, point.y);
	}

	void Win32Input::SetCursorVisible(bool visible) const
	{
		ShowCursor(visible);
		m_isCursorVisible = visible;
	}

	bool Win32Input::IsCursorVisible() const
	{
		return m_isCursorVisible;
	}

} }
