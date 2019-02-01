#pragma once

#include "../Math/Math.h"

#include "InputDefs.h"

#define INPUT_MAX_KEYS 256
#define INPUT_MAX_MOUSE_BUTTONS 5

namespace Happic { namespace Core {

	class IDisplayInput
	{
	public:
		IDisplayInput();

		virtual Math::Vector2f GetCursorPosition() const = 0;
		virtual void SetCursorPosition(const Math::Vector2f& pos) const = 0;
		virtual void SetCursorVisible(bool hidden) const = 0;
		virtual bool IsCursorVisible() const = 0;

		void Update();

		void SetKeyState(Key key, bool state);
		void SetMouseButtonState(MouseButton button, bool state);

		bool IsKeyDown(Key key) const;
		bool IsKeyPressed(Key key) const;
		bool IsKeyReleased(Key key) const;

		bool IsMouseButtonDown(MouseButton button) const;
		bool IsMouseButtonPressed(MouseButton button) const;
		bool IsMouseButtonReleased(MouseButton button) const;
	private:
		bool m_keys[INPUT_MAX_KEYS];
		bool m_lastKeys[INPUT_MAX_KEYS];
		bool m_buttons[INPUT_MAX_MOUSE_BUTTONS];
		bool m_lastButtons[INPUT_MAX_MOUSE_BUTTONS];
	};

} }

