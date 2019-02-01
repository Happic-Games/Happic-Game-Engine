#include "IDisplayInput.h"
#include <cstring>

namespace Happic { namespace Core {

	IDisplayInput::IDisplayInput()
	{
		memset(m_keys, false, INPUT_MAX_KEYS * sizeof(bool));
		memset(m_lastKeys, false, INPUT_MAX_KEYS * sizeof(bool));
		memset(m_buttons, false, INPUT_MAX_MOUSE_BUTTONS * sizeof(bool));
		memset(m_lastButtons, false, INPUT_MAX_MOUSE_BUTTONS * sizeof(bool));
	}

	void IDisplayInput::Update()
	{
		memcpy(m_lastKeys, m_keys, INPUT_MAX_KEYS * sizeof(bool));
		memcpy(m_lastButtons, m_buttons, INPUT_MAX_MOUSE_BUTTONS * sizeof(bool));
	}

	void IDisplayInput::SetKeyState(Key key, bool state)
	{
		m_keys[key] = state;
	}

	void IDisplayInput::SetMouseButtonState(MouseButton button, bool state)
	{
		m_buttons[button] = state;
	}

	bool IDisplayInput::IsKeyDown(Key key) const
	{
		return m_keys[key];
	}

	bool IDisplayInput::IsKeyPressed(Key key) const
	{
		return m_keys[key] && !m_lastKeys[key];
	}

	bool IDisplayInput::IsKeyReleased(Key key) const
	{
		return !m_keys[key] && m_lastKeys[key];
	}

	bool IDisplayInput::IsMouseButtonDown(MouseButton button) const
	{
		return m_buttons[button];
	}

	bool IDisplayInput::IsMouseButtonPressed(MouseButton button) const
	{
		return m_buttons[button] && !m_lastButtons[button];
	}

	bool IDisplayInput::IsMouseButtonReleased(MouseButton button) const
	{
		return !m_buttons[button] && m_lastButtons[button];
	}

} }
