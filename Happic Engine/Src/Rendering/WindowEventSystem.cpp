#include "WindowEventSystem.h"

namespace Happic { namespace Rendering {

	WindowEventInfo WindowEventSystem::s_windowEventInfos[];
	std::vector<WindowEventAction> WindowEventSystem::s_windowEventActions[];

	const WindowEventInfo & WindowEventSystem::GetWindowEventInfo(WindowEvent windowEvent)
	{
		return s_windowEventInfos[windowEvent];
	}

	bool WindowEventSystem::CheckForWindowEvent(WindowEvent windowEvent)
	{
		return s_windowEventInfos[windowEvent].occurred;
	}

	void Rendering::WindowEventSystem::WindowEventOccurred(WindowEvent windowEvent, const WindowEventInfo & windowEventInfo)
	{
		s_windowEventInfos[windowEvent] = windowEventInfo;
		for (uint32 i = 0; i < s_windowEventActions[windowEvent].size(); i++)
		{
			bool changed = s_windowEventActions[windowEvent][i].function(windowEvent, windowEventInfo, s_windowEventActions[windowEvent][i].pUserPtr);
		}
	}

	void WindowEventSystem::ResetWindowEvents()
	{
		for (uint32 i = 0; i < NUM_WINDOW_EVENTS; i++)
			s_windowEventInfos[i].occurred = false;
	}

	void WindowEventSystem::AddWindowEventAction(WindowEvent windowEvent, const WindowEventAction & windowEventAction)
	{
		s_windowEventActions[windowEvent].push_back(windowEventAction);
	}

} }