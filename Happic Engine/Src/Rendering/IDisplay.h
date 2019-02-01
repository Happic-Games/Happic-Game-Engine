#pragma once

#include "../DataStructures/Types.h"
#include "../DataStructures/String.h"
#include "RenderAPI.h"
#include "WindowEventSystem.h"
#include "../Core/IDisplayInput.h"

namespace Happic { namespace Rendering {

	enum DisplayType
	{
		DISPLAY_TYPE_WIN_32
	};

	struct DisplayError
	{
		DisplayType displayType;
		String errMsg;
		bool success;
	};

	class IDisplay
	{
	public:
		virtual DisplayError Init(RenderAPI renderAPI) = 0;

		virtual void GetSize(uint32* pWidth, uint32* pHeight) const = 0;

		virtual void Update() = 0;

		virtual Core::IDisplayInput* GetInput() const = 0;
	public:
		static IDisplay* CreateDisplay(const String& title, uint32 width, uint32 height);
	};

} }
