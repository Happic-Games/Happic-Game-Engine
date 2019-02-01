#pragma once

#include <vector>
#include "../DataStructures/Types.h"

namespace Happic { namespace Rendering {

	enum WindowEvent
	{
		WINDOW_EVENT_RESIZE,
		WINDOW_EVENT_CLOSED,
		NUM_WINDOW_EVENTS
	};

	struct WindowEventInfo
	{
		WindowEventInfo(bool occurred = false, uint32 width = 0, uint32 height = 0) :
			occurred(occurred),
			width(width),
			height(height)
		{}

		bool occurred;
		uint32 width;
		uint32 height;
	};

	typedef bool(*window_event_action_function)(WindowEvent windowEvent, const WindowEventInfo& windowEventInfo, void* pUserPtr);

	struct WindowEventAction
	{
		void* pUserPtr;
		window_event_action_function function;
	};

	typedef uint32 WindowEventActionID;

	class WindowEventSystem
	{
	public:
		static const WindowEventInfo& GetWindowEventInfo(WindowEvent windowEvent);
		static bool CheckForWindowEvent(WindowEvent windowEvent);
		static void WindowEventOccurred(WindowEvent windowEvent, const WindowEventInfo& windowEventInfo);
		static void ResetWindowEvents();
		static void AddWindowEventAction(WindowEvent windowEvent, const WindowEventAction& windowEventAction);

	private:
		static WindowEventInfo s_windowEventInfos[NUM_WINDOW_EVENTS];
		static std::vector<WindowEventAction> s_windowEventActions[NUM_WINDOW_EVENTS];
	};

} }
