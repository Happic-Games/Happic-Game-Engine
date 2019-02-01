#include "IDisplay.h"

#include "../Platform/Windows/Win32Display.h"

namespace Happic { namespace Rendering { 

	IDisplay * IDisplay::CreateDisplay(const String& title, uint32 width, uint32 height)
	{
#ifdef _WIN32
		return new Win32Display(title, width, height);
#endif
	}

} }
