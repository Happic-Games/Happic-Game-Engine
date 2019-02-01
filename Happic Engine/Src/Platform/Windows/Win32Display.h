#pragma once

#include "../../Rendering/IDisplay.h"
#include "Win32Input.h"

//#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Happic { namespace Rendering {

	class Win32Display : public IDisplay
	{
	public:
		Win32Display(const String& title, uint32 width, uint32 height);
		~Win32Display();

		DisplayError Init(RenderAPI renderAPI) override;
		void Update() override;

		void GetSize(uint32* pWidth, uint32* pHeight) const override;

		Core::IDisplayInput* GetInput() const override;

		HWND GetWindowHandle() const;
	private:
		bool InitRawInputs();
		friend void handle_input(Core::IDisplayInput* pDisplayInput, WPARAM wparam, LPARAM lparam);
		friend LRESULT CALLBACK WinProc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam);
	private:
		HWND m_windowHandle;

		Core::Win32Input* m_pInput;

		String m_initTitle;
		uint32 m_initWidth;
		uint32 m_initHeight;
	};

} }
