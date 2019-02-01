#include "Win32Display.h"

namespace Happic { namespace Rendering { 

	Win32Display::Win32Display(const String & title, uint32 width, uint32 height) :
		m_initTitle(title),
		m_initWidth(width),
		m_initHeight(height)
	{
		
	}

	Win32Display::~Win32Display()
	{
		DestroyWindow(m_windowHandle);
	}

	DisplayError Win32Display::Init(RenderAPI renderAPI)
	{
		DisplayError err;
		err.displayType = DISPLAY_TYPE_WIN_32;
		err.errMsg = "Success";
		err.success = true;

		WNDCLASS wnd_class = { 0 };
		wnd_class.style = CS_OWNDC;
		wnd_class.lpfnWndProc = WinProc;
		wnd_class.hCursor = LoadCursor(NULL, IDC_ARROW);
		wnd_class.lpszClassName = "happic";
		RegisterClass(&wnd_class);

		SetProcessDPIAware();

		uint32 monitorWidth = GetSystemMetrics(SM_CXSCREEN);
		uint32 monitorHeight = GetSystemMetrics(SM_CYSCREEN);

		uint32 xpos = (monitorWidth - m_initWidth) / 2;
		uint32 ypos = (monitorHeight - m_initHeight) / 2;

		String title = m_initTitle + "  |  ";
		switch (renderAPI)
		{
		case RENDER_API_VULKAN: title += "Render API : Vulkan"; break;
		case RENDER_API_OPENGL: title += "Render API : Opengl"; break;
		case RENDER_API_D3D11: title += "Render API : Direct3D 11"; break;
		}

		title += "  |  FPS : ";

		int flags = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;
		m_windowHandle = CreateWindow(wnd_class.lpszClassName, title.C_Str(), flags, xpos, ypos, m_initWidth, m_initHeight, NULL, NULL, NULL, this);
	
		if (!InitRawInputs())
		{
			err.displayType = DISPLAY_TYPE_WIN_32;
			err.errMsg = "Could not create Win32 raw inputs";
			err.success = false;
			return err;
		}

		PIXELFORMATDESCRIPTOR pixel_format{};

		pixel_format.dwFlags = PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;

		if (renderAPI == RENDER_API_OPENGL)
		{
			pixel_format.dwFlags |= PFD_SUPPORT_OPENGL;
		}

		pixel_format.nVersion = 1;
		pixel_format.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pixel_format.cColorBits = 32;
		pixel_format.cRedBits = 8;
		pixel_format.cGreenBits = 8;
		pixel_format.cBlueBits = 8;
		pixel_format.cAlphaBits = 8;
		pixel_format.cDepthBits = 24;
		pixel_format.cStencilBits = 8;
		pixel_format.iPixelType = PFD_TYPE_RGBA;
		pixel_format.cAuxBuffers = 0;
		pixel_format.iLayerType = PFD_MAIN_PLANE;

		HDC hdc = GetDC(m_windowHandle);

		int pixelFormat = ChoosePixelFormat(hdc, &pixel_format);

		if (pixelFormat)
		{
			if (!SetPixelFormat(hdc, pixelFormat, &pixel_format))
			{
				err.errMsg = "Could not set pixel format for Win32";
				err.success = false;
				return err;
			}
		}
		else
		{
			err.errMsg = "Could not set pixel format for Win32";
			err.success = false;
			return err;
		}

		m_pInput = new Core::Win32Input(m_windowHandle);

		return err;
	}

	void Win32Display::Update()
	{
		MSG msg = { 0 };
		while (PeekMessage(&msg, m_windowHandle, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void Win32Display::GetSize(uint32 * pWidth, uint32 * pHeight) const
	{
		RECT rect;
		if (GetWindowRect(m_windowHandle, &rect))
		{
			*pWidth = (uint32)(rect.right - rect.left);
			*pHeight = (uint32)(rect.bottom - rect.top);
		}
	}

	Core::IDisplayInput * Win32Display::GetInput() const
	{
		return m_pInput;
	}

	HWND Win32Display::GetWindowHandle() const
	{
		return m_windowHandle;
	}

	LRESULT CALLBACK WinProc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		Win32Display* pDisplay = (Win32Display*)GetWindowLongPtr(handle, GWL_USERDATA);

		switch (msg)
		{
		case WM_CLOSE: WindowEventSystem::WindowEventOccurred(WINDOW_EVENT_CLOSED, WindowEventInfo(true)); break;
		case WM_NCCREATE: SetWindowLongPtr(handle, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lparam)->lpCreateParams);
			SetWindowPos(handle, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER); 
			break;
		case WM_SIZE: WindowEventSystem::WindowEventOccurred(WINDOW_EVENT_RESIZE, WindowEventInfo(true, LOWORD(lparam), HIWORD(lparam))); break;
		case WM_INPUT: handle_input(pDisplay->m_pInput, wparam, lparam); break;
		}

		return DefWindowProc(handle, msg, wparam, lparam);
	}

	bool Win32Display::InitRawInputs()
	{
		RAWINPUTDEVICE raw_input_keyboard_device{};

		raw_input_keyboard_device.dwFlags = RIDEV_NOLEGACY;
		raw_input_keyboard_device.usUsagePage = 0x01;
		raw_input_keyboard_device.usUsage = 0x06;
		raw_input_keyboard_device.hwndTarget = m_windowHandle;

		RAWINPUTDEVICE raw_input_mouse_device{};

		raw_input_mouse_device.dwFlags = 0;
		raw_input_mouse_device.usUsagePage = 0x01;
		raw_input_mouse_device.usUsage = 0x02;
		raw_input_mouse_device.hwndTarget = m_windowHandle;

		RAWINPUTDEVICE raw_input_device_list[] = { raw_input_mouse_device, raw_input_keyboard_device };

		return RegisterRawInputDevices(raw_input_device_list, 2, sizeof(RAWINPUTDEVICE));
	}

	void handle_input(Core::IDisplayInput* pDisplayInput, WPARAM wparam, LPARAM lparam)
	{
		RAWINPUT* pInput;
		UINT size;
		LPBYTE bytes;

		GetRawInputData((HRAWINPUT)lparam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));

		bytes = new BYTE[size];

		if (!bytes)
		{
			std::cerr << "Error getting windows raw input data" << std::endl;
			return;
		}

		if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT, bytes, &size, sizeof(RAWINPUTHEADER)) != size)
		{

		}

		pInput = (RAWINPUT*)bytes;

		if (pInput->header.dwType == RIM_TYPEKEYBOARD)
		{
			if (pInput->data.keyboard.Message == WM_KEYDOWN)
			{
				USHORT key = pInput->data.keyboard.MakeCode;
				pDisplayInput->SetKeyState(key, true);
			}
			else if (pInput->data.keyboard.Message == WM_KEYUP)
			{
				USHORT key = pInput->data.keyboard.MakeCode;
				pDisplayInput->SetKeyState(key, false);
			}
		}
		else if (pInput->header.dwType == RIM_TYPEMOUSE)
		{
			const RAWMOUSE& mouse = pInput->data.mouse;

			USHORT wheel = mouse.usButtonData;
			USHORT button = mouse.ulButtons;

			unsigned int engineButton = 0;
			int buttonStatus = -1;

			switch (button)
			{
			case RI_MOUSE_BUTTON_1_DOWN:	engineButton = 0; buttonStatus = true; break;
			case RI_MOUSE_BUTTON_1_UP:		engineButton = 0; buttonStatus = false; break;
			case RI_MOUSE_BUTTON_2_DOWN:	engineButton = 1; buttonStatus = true; break;
			case RI_MOUSE_BUTTON_2_UP:		engineButton = 1; buttonStatus = false; break;
			case RI_MOUSE_BUTTON_3_DOWN:	engineButton = 2; buttonStatus = true; break;
			case RI_MOUSE_BUTTON_3_UP:		engineButton = 2; buttonStatus = false; break;
			case RI_MOUSE_BUTTON_4_DOWN:	engineButton = 3; buttonStatus = true; break;
			case RI_MOUSE_BUTTON_4_UP:		engineButton = 3; buttonStatus = false; break;
			case RI_MOUSE_BUTTON_5_DOWN:	engineButton = 4; buttonStatus = true; break;
			case RI_MOUSE_BUTTON_5_UP:		engineButton = 4; buttonStatus = false; break;
			case RI_MOUSE_WHEEL:			engineButton = 5; buttonStatus = true; break;
			}

			if (buttonStatus != -1)
			{
				pDisplayInput->SetMouseButtonState(engineButton, buttonStatus);
			}
		}

		delete bytes;
	}

} }
