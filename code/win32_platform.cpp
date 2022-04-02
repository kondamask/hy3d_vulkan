#include "win32_platform.h"
#include "hy3d_vulkan.cpp"

// NOTE: These file I/O functions should only be used for DEBUG purposes.
DEBUG_FREE_FILE(DEBUGFreeFileMemory)
{
	if (memory)
	{
		VirtualFree(memory, 0, MEM_RELEASE);
	}
}

DEBUG_READ_FILE(DEBUGReadFile)
{
	debug_read_file_result result = {};

	char fullFilePath[MAX_PATH] = {};
	if (GetFullPathNameA(filename, ArrayCount(fullFilePath), fullFilePath, 0) == 0)
		return result;

	HANDLE fileHandle = CreateFileA(fullFilePath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fileSize;
		if (GetFileSizeEx(fileHandle, &fileSize))
		{// Truncate 64 bit value to 32 bit because VirtualAlloc only takes 32bit value
			Assert(fileSize.QuadPart <= 0xFFFFFFFF);
			result.size = (uint32_t)fileSize.QuadPart;

			result.content = VirtualAlloc(0, result.size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			if (result.content)
			{
				DWORD bytesRead;
				if (ReadFile(fileHandle, result.content, result.size, &bytesRead, 0) &&
					result.size == bytesRead)
				{
					// We read the file successfully
				}
				else
				{
					DEBUGFreeFileMemory(result.content);
					result = {};
				}
			}
		}
		CloseHandle(fileHandle);
	}
	// NOTE:  We can add logging in case these steps fail.
	return result;
}

DEBUG_WRITE_FILE(DEBUGWriteFile)
{
	bool result = false;

	char fullFilePath[MAX_PATH] = {};
	if (GetFullPathNameA(filename, ArrayCount(fullFilePath), fullFilePath, 0) == 0)
		return result;

	HANDLE fileHandle = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD bytesWritten;
		if (WriteFile(fileHandle, memory, memorySize, &bytesWritten, 0))
		{
			// We wrote into the file successfully
			result = (bytesWritten == memorySize);
		}
		CloseHandle(fileHandle);
	}
	// NOTE:  We can add logging in case these steps fail.
	return result;
}

static_func LRESULT CALLBACK Win32MainWindowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
	// NOTE: This pointer doesn't need to be checked for null since it always gets a value
	// before we need to process ather messages. On application start we get:
	// 1st message: WM_GETMINMAXINFO
	// 2nd message: WM_NCCREATE -> sets window pointer in the windows api
	win32_window *window = (win32_window *)GetWindowLongPtr(handle, GWLP_USERDATA);

	//		 before the other messages.
	LRESULT result = 0;
	switch (message)
	{
		case WM_PAINT:
		{
			ValidateRect(handle, 0);
			break;
		}
		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
			lpMMI->ptMinTrackSize.x = WINDOW_WIDTH_MIN + WIN32_WINDOW_X_BORDER;
			lpMMI->ptMinTrackSize.y = WINDOW_HEIGHT_MIN + WIN32_WINDOW_Y_BORDER;
			break;
		}
		case WM_SIZE:
		{
			PostMessage(handle, WM_USER + 1, wParam, lParam);
			break;
		}
		//case WM_EXITSIZEMOVE:
		case WM_CLOSE:
		{
			UnregisterClassA(window->name, window->instance);
			DestroyWindow(handle);
			break;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}

		case WM_NCCREATE:
		{
			CREATESTRUCT *pCreate = (CREATESTRUCT *)lParam;
			if (pCreate)
			{
				win32_window *pWindow = (win32_window *)(pCreate->lpCreateParams);
				// Set WinAPI-managed user data to store ptr to window class
				SetWindowLongPtr(handle, GWLP_USERDATA, (LONG_PTR)(pWindow));
			}
		}

		// 	NOTE: KEYBOARD AND MOUSE EVENTS SHOULD NOT COME HERE!
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYUP:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_MOUSELEAVE:
		case WM_KILLFOCUS:
		{
			Assert("We got an input message from somewhere else and we did not handle it properly");
		}

		default:
			result = DefWindowProc(handle, message, wParam, lParam);
	}
	return result;
}

static_func void Win32Update(win32_window &window)
{
	return;
}

static_func KEYBOARD_BUTTON Win32TranslateKeyInput(VK_CODE code)
{
	switch (code)
	{
		case VK_UP: { return KEY_UP ; }
		break;
		case VK_LEFT: { return KEY_LEFT ; }
		break;
		case VK_DOWN: { return KEY_DOWN ; }
		break;
		case VK_RIGHT: { return KEY_RIGHT ; }
		break;
		case 0x57: { return KEY_W ; }
		break;
		case 0x41: { return KEY_A ; }
		break;
		case 0x53: { return KEY_S ; }
		break;
		case 0x44: { return KEY_D ; }
		break;
		case 0x51: { return KEY_Q ; }
		break;
		case 0x45: { return KEY_E ; }
		break;
		case 0x52: { return KEY_R ; }
		break;
		case 0x46: { return KEY_F ; }
		break;
		case 0x5A: { return KEY_Z ; }
		break;
		case 0x58: { return KEY_X ; }
		break;
		case 0x43: { return KEY_C ; }
		break;
		case 0x56: { return KEY_V ; }
		break;
		case 0x49: { return KEY_I ; }
		break;
		case 0x4A: { return KEY_J ; }
		break;
		case 0x4B: { return KEY_K ; }
		break;
		case 0x4C: { return KEY_L ; }
		break;
		case 0x55: { return KEY_U ; }
		break;
		case 0x4F: { return KEY_O ; }
		break;
		case VK_SHIFT: { return KEY_SHIFT;  }
		break;
		case VK_CONTROL: { return KEY_CTRL ; }
		break;
		case VK_MENU: { return KEY_ALT ; }
		break;
		case VK_SPACE: { return KEY_SPACE ; }
		break;
		case VK_F4: { return KEY_F4 ; }
		break;
		case VK_OEM_3: { return KEY_TILDE ; }
		break;
		case 0x30: { return KEY_ZERO ; }
		break;
		case 0x31: { return KEY_ONE ; }
		break;
		case 0x32: { return KEY_TWO ; }
		break;
		case 0x33: { return KEY_THREE ; }
		break;
		case 0x34: { return KEY_FOUR ; }
		break;
		case 0x35: { return KEY_FIVE ; }
		break;
		case 0x36: { return KEY_SIX ; }
		break;
		case 0x37: { return KEY_SEVEN ; }
		break;
		case 0x38: { return KEY_EIGHT ; }
		break;
		case 0x39: { return KEY_NINE ; }
		break;
		default: { return KEY_INVALID ; }
		break;
	}
}

static_func wnd_dim Win32GetWindowDim(HWND handle)
{
	wnd_dim result = {};
	RECT rect = {};
	GetWindowRect(handle, &rect);
	result.width = (i16)(rect.right - rect.left);
	result.height = (i16)(rect.bottom - rect.top);
	return result;
}

static_func bool Win32ProcessMessages(win32_window &window, engine_input &input, i32 &quitMessage)
{
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		quitMessage = (i32)message.wParam;

		switch (message.message)
		{
			case WM_QUIT:
			{
				return false;
				break;
			}

			case WM_USER + 1: //WM_SIZE
			{
				window.onResize = true;
				window.dimensions = Win32GetWindowDim(window.handle);
				break;
			}

            /***************** KEYBOARD EVENTS ****************/
			case WM_SYSKEYDOWN:
			case WM_KEYDOWN:
			{
				bool wasDown = ((message.lParam >> 30) & 1) != 0;
				if (!wasDown || input.keyboard.autoRepeatEnabled)
				{
					KEYBOARD_BUTTON key = Win32TranslateKeyInput((VK_CODE)message.wParam);
					if (key < KEY_COUNT)
						input.keyboard.ToggleKey(key);
				}
				if (input.keyboard.isPressed[KEY_F4] && input.keyboard.isPressed[KEY_ALT])
				{
					PostQuitMessage(0);
					return 0;
				}
				break;
			}
			case WM_SYSKEYUP:
			case WM_KEYUP:
			{
				KEYBOARD_BUTTON key = Win32TranslateKeyInput((VK_CODE)message.wParam);
				if (key < KEY_COUNT)
					input.keyboard.ToggleKey(key);
				break;
			}
            /**************************************************/

            /****************** MOUSE EVENTS ******************/
			case WM_INPUT:
			{
				UINT dwSize = sizeof(RAWINPUT);
				local_var BYTE lpb[sizeof(RAWINPUT)];
				if (GetRawInputData((HRAWINPUT)message.lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) == -1)
				{
					break;
				}

				RAWINPUT* raw = (RAWINPUT*)lpb;
				if (raw->header.dwType == RIM_TYPEMOUSE &&
					(raw->data.mouse.lLastX != 0 || raw->data.mouse.lLastY != 0))
				{
					input.mouse.newPos.X += (f32)raw->data.mouse.lLastX;
					input.mouse.newPos.Y += (f32)raw->data.mouse.lLastY;
				}
				break;
			}
			case WM_MOUSEMOVE:
			{
				POINTS p = MAKEPOINTS(message.lParam);
				p.y = window.dimensions.height - p.y;
				bool isInWindow =
					p.x >= 0 && p.x < window.dimensions.width &&
					p.y >= 0 && p.y < window.dimensions.height;
				if (input.mouse.cursorEnabled)
				{
					while (ShowCursor(TRUE) < 0);
					ClipCursor(0);
				}
				else
				{
					while (ShowCursor(FALSE) >= 0);
					RECT rect;
					GetClientRect(window.handle, &rect);
					MapWindowPoints(window.handle, 0, (POINT*)(&rect), 2);
					ClipCursor(&rect);
				}
				if (isInWindow)
				{
					//ShowCursor(false);
					//input.mouse.SetPos({(f32)p.x, (f32)p.y});
					if (!input.mouse.isInWindow) // if it wasn't in the window before
					{
						SetCapture(window.handle);
						input.mouse.isInWindow = true;
					}
				}
				else
				{
					//ShowCursor(true);
					if (!(input.mouse.leftIsPressed || input.mouse.rightIsPressed))
					{
						//ReleaseCapture();
						//input.mouse.isInWindow = false;
					}
				}
				break;
			}
			case WM_LBUTTONDOWN:
				input.mouse.leftIsPressed = true;
				break;
			case WM_RBUTTONDOWN:
				input.mouse.rightIsPressed = true;
				break;
			case WM_LBUTTONUP:
				input.mouse.leftIsPressed = false;
				break;
			case WM_RBUTTONUP:
				input.mouse.rightIsPressed = false;
				break;
			case WM_MOUSEWHEEL:
				input.mouse.SetWheelDelta(GET_WHEEL_DELTA_WPARAM(message.wParam));
			case WM_MOUSELEAVE:
				POINTS p = MAKEPOINTS(message.lParam);
				//input.mouse.SetPos( { (f32)p.x, (f32)p.y });
				break;
			case WM_KILLFOCUS:
				input.keyboard.Clear();
				break;
			default:
				TranslateMessage(&message);
				DispatchMessage(&message);
				break;
		}
	}
	return true;
}

static_func FILETIME Win32GetWriteTime(char *filename)
{
	FILETIME result = {};
	char fullFilePath[MAX_PATH] = {};
	if (GetFullPathNameA(filename, ArrayCount(fullFilePath), fullFilePath, 0) == 0)
		return result;

	WIN32_FIND_DATA data = {};
	HANDLE handle = FindFirstFileA(fullFilePath, &data);
	if (handle != INVALID_HANDLE_VALUE)
	{
		result = data.ftLastWriteTime;
		FindClose(handle);
	}
	return result;
}

static_func inline bool Win32FileUpdated(char *filename, FILETIME &oldWriteTime)
{
	FILETIME newWriteTime = Win32GetWriteTime(filename);
	return (CompareFileTime(&newWriteTime, &oldWriteTime) == 1);
}

static_func void Win32LoadEngineCode(win32_engine_code *engineCode, char *sourceFilename, char *sourceFilenameCopy)
{
	// NOTE:  We need to add a sleep in order to wait for the dll compilation.
	Sleep(800);
	char sourceFullPath[MAX_PATH] = {};
	if (GetFullPathNameA(sourceFilename, ArrayCount(sourceFullPath), sourceFullPath, 0) == 0)
		return;
	char sourceFullPathCopy[MAX_PATH] = {};
	if (GetFullPathNameA(sourceFilenameCopy, ArrayCount(sourceFullPathCopy), sourceFullPathCopy, 0) == 0)
		return;

	engineCode->writeTime = Win32GetWriteTime(sourceFullPath);
	CopyFileA(sourceFullPath, sourceFullPathCopy, FALSE);
	engineCode->dll = LoadLibraryA(sourceFullPathCopy);
	if (engineCode->dll)
	{
		engineCode->UpdateAndRender = (update_and_render *)GetProcAddress(engineCode->dll, "UpdateAndRender");
		engineCode->isValid = engineCode->UpdateAndRender;
	}
	if (!engineCode->isValid)
	{
		engineCode->UpdateAndRender = UpdateAndRenderStub;
	}
}

static_func void Win32UnloadEngineCode(win32_engine_code *engineCode)
{
	if (engineCode->dll)
	{
		FreeLibrary(engineCode->dll);
		engineCode->dll = 0;
	}
	engineCode->isValid = false;
	engineCode->UpdateAndRender = UpdateAndRenderStub;
}

static_func bool Win32InitializeWindow(win32_window &window, u16 width, u16 height, LPCSTR windowTitle)
{
#if HY3D_DEBUG
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout);
	freopen("CON", "w", stderr);
	SetConsoleTitle("hy3d vulkan log");
#endif

	window.instance = GetModuleHandleW(nullptr);

	// Set window class properties
	WNDCLASSA windowClass = {};
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = Win32MainWindowProc;
	windowClass.lpszClassName = windowTitle;
	windowClass.hInstance = window.instance;
	windowClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	windowClass.hIcon = LoadIconA(windowClass.hInstance, MAKEINTRESOURCEA(IDI_MYAPP_ICON));
	//windowClass.hCursor = LoadCursorA(0, IDC_CROSS);

	if (!RegisterClassA(&windowClass))
	{
		DebugPrint("ERROR: Window class wasn't registered.\n");
		return false;
	}

	window.dimensions.width = width;
	window.dimensions.height = height;
	// Declare the window client size
	RECT rect = {};
	rect.left = 100;
	rect.top = 100;
	rect.right = rect.left + window.dimensions.width;
	rect.bottom = rect.top + window.dimensions.height;

	// Adjuct the window size according to the style we
	// have for out window, while keeping the client size
	// the same.
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, FALSE);
	window.dimensions.width = (i16)(rect.right - rect.left);
	window.dimensions.height = (i16)(rect.bottom - rect.top);

	// Create the window
	window.handle = CreateWindowA(
		windowTitle,
		windowTitle,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, // X, Y
		window.dimensions.width,
		window.dimensions.height,
		nullptr, nullptr,
		window.instance,
		&window // * See note bellow
	);

	if (!window.handle)
	{
		DebugPrint("ERROR: Failed to create window.\n");
		return false;
	}

	RAWINPUTDEVICE rid;
	rid.usUsagePage = 0x01; //HID_USAGE_PAGE_GENERIC; 
	rid.usUsage = 0x02; //HID_USAGE_GENERIC_MOUSE; 
	//rid.dwFlags = RIDEV_INPUTSINK;   
	rid.hwndTarget = window.handle;
	RegisterRawInputDevices(&rid, 1, sizeof(rid));

	ShowWindow(window.handle, SW_SHOWDEFAULT);
	return true;
}

#include <strsafe.h>
void ErrorExit(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw);
}

static_func bool Win32InitializeMemory(engine_memory &memory)
{
	LPVOID baseAddress = (LPVOID)TERABYTES(2);
	memory.permanentMemorySize = MEGABYTES(64);
	memory.transientMemorySize = GIGABYTES(2);
	u64 totalSize = memory.permanentMemorySize + memory.transientMemorySize;
	memory.permanentMemory = VirtualAlloc(baseAddress, (SIZE_T)totalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	memory.transientMemory = (u8 *)memory.permanentMemory + memory.permanentMemorySize;

	memory.stagingMemory = vulkan.stagingBuffer.data;

	memory.isInitialized = false;

	memory.platformAPI_.Draw = Vulkan::Draw;
	memory.platformAPI_.PushStaged = Vulkan::PushStaged;
	memory.platformAPI_.ChangeGraphicsSettings = Vulkan::ChangeGraphicsSettings;

	//#if HY3D_DEBUG
	memory.platformAPI_.DEBUGFreeFileMemory = DEBUGFreeFileMemory;
	memory.platformAPI_.DEBUGReadFile = DEBUGReadFile;
	memory.platformAPI_.DEBUGWriteFile = DEBUGWriteFile;
	//#endif

	platformAPI = memory.platformAPI_;
	return (memory.permanentMemory && memory.transientMemory);
}

int CALLBACK WinMain(
	HINSTANCE instance,
	HINSTANCE prevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	win32_window window = {};

	if (!Win32InitializeWindow(window, WINDOW_WIDTH, WINDOW_HEIGHT, window.name))
	{
		return 1;
	}

	// TODO: Use this to load OBJs
	/*char filename[FILENAME_MAX] = {};
	WORD filenameOffset = 0;
	WORD extensionOffset = 0;
	OPENFILENAMEA openFilename = {};
	openFilename.lStructSize = sizeof(OPENFILENAME);
	openFilename.hwndOwner = window.handle;
	openFilename.hInstance = window.instance;
	openFilename.lpstrFile = filename;
	openFilename.lpstrFilter = "Wavefront .obj file\0*.obj\0\0";
	openFilename.nMaxFile = ArrayCount(filename);
	openFilename.lpstrTitle = "Select an OBJ file";
	openFilename.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	openFilename.nFileOffset = filenameOffset;
	openFilename.nFileExtension = extensionOffset;
	openFilename.lpstrDefExt = "obj";
	GetOpenFileNameA(&openFilename);*/

	win32_engine_code engineCode = {};
	// TODO: make this less explicit
	char *sourceDLLPath = "build\\hy3d_engine.dll";
	char *sourceDLLCopyPath = "build\\hy3d_engine_copy.dll";
	Win32LoadEngineCode(&engineCode, sourceDLLPath, sourceDLLCopyPath);

	engine_context engine = {};
	if (!Win32InitializeMemory(engine.memory))
	{
		return 3;
	}

	if (!Vulkan::Win32Initialize(window.instance, window.handle, window.name))
	{
		return 4;
	}
	engine.memory.stagingMemory = vulkan.stagingBuffer.data;

	// TODO(heyyod): This assumes that the first image we aquire in vulkan will always have index 0
	// Mayby bad
	engine.memory.cameraData = (camera_data *)vulkan.frameData[0].cameraBuffer.data;
	engine.memory.sceneData = (scene_data *)vulkan.frameData[0].sceneBuffer.data;
	engine.memory.objectsTransforms = (object_transform *)vulkan.staticTransformsBuffer.data;

	FILETIME shadersWriteTime = Win32GetWriteTime(shaderFiles[0]);
	i32 quitMessage = -1;
	while (Win32ProcessMessages(window, engine.input, quitMessage))
	{
		if (Win32FileUpdated(sourceDLLPath, engineCode.writeTime))
		{
			Win32UnloadEngineCode(&engineCode);
			Win32LoadEngineCode(&engineCode, sourceDLLPath, sourceDLLCopyPath);
		}
		if (Win32FileUpdated(shaderFiles[0], shadersWriteTime))
		{
			shadersWriteTime = Win32GetWriteTime(shaderFiles[0]);
			if (!Vulkan::CreatePipeline())
			{
				return 5;
			}
		}
		if (window.onResize)
		{
			if (!Vulkan::CreateSwapchain())
			{
				return 6;
			}
			window.onResize = false;
			engine.windowWidth = vulkan.windowExtent.width;
			engine.windowHeight = vulkan.windowExtent.height;
		}
		if (vulkan.canRender)
			engineCode.UpdateAndRender(&engine);
		Win32Update(window);
	}
	Vulkan::Destroy();
	return quitMessage;
}