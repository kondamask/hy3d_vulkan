#include "win32_platform.h"

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
	HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fileSize;
		if (GetFileSizeEx(fileHandle, &fileSize))
		{
			// Truncate 64 bit value to 32 bit because VirtualAlloc only takes 32bit value
			ASSERT(fileSize.QuadPart <= 0xFFFFFFFF);
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

static void Win32DestroyVulkan(win32_vulkan_state &vulkan)
{
	vkDestroyImage(vulkan.device, vulkan.depthImage, 0);
	for (uint32_t i = 0; i < vulkan.swapchainImageCount; i++)
	{
		vkDestroyImageView(vulkan.device, vulkan.imageViews[i], 0);
	}
	vkDestroySwapchainKHR(vulkan.device, vulkan.swapchain, 0);
	vkDestroyCommandPool(vulkan.device, vulkan.cmdPool, 0);
	vkDeviceWaitIdle(vulkan.device);
	vkDestroyDevice(vulkan.device, 0);
	vkDestroySurfaceKHR(vulkan.instance, vulkan.surface, 0);

#if VULKAN_VALIDATION_LAYERS_ON
	PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugMessenger =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkan.instance, "vkDestroyDebugUtilsMessengerEXT");
	if (destroyDebugMessenger != nullptr)
	{
		destroyDebugMessenger(vulkan.instance, vulkan.debugMessenger, 0);
	}
#endif

	vkDestroyInstance(vulkan.instance, 0);
}

static LRESULT Win32MainWindowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
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

	case WM_CLOSE:
	{
		UnregisterClassA(window->name, window->instance);
		Win32DestroyVulkan(window->vulkan);
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
		ASSERT("We got an input message from somewhere else and we did not handle it properly");
	}

	default:
		result = DefWindowProc(handle, message, wParam, lParam);
	}
	return result;
}

static void Win32Update(win32_window &window)
{
	return;
}

static KEYBOARD_BUTTON Win32TranslateKeyInput(VK_CODE code)
{
	switch (code)
	{
	case VK_UP:
		return UP;
		break;
	case VK_LEFT:
		return LEFT;
		break;
	case VK_DOWN:
		return DOWN;
		break;
	case VK_RIGHT:
		return RIGHT;
		break;
	case 0x57:
		return W;
		break;
	case 0x41:
		return A;
		break;
	case 0x53:
		return S;
		break;
	case 0x44:
		return D;
		break;
	case 0x51:
		return Q;
		break;
	case 0x45:
		return E;
		break;
	case 0x52:
		return R;
		break;
	case 0x46:
		return F;
		break;
	case 0x5A:
		return Z;
		break;
	case 0x58:
		return X;
		break;
	case 0x43:
		return C;
		break;
	case 0x56:
		return V;
		break;
	case 0x49:
		return I;
		break;
	case 0x4A:
		return J;
		break;
	case 0x4B:
		return K;
		break;
	case 0x4C:
		return L;
		break;
	case 0x55:
		return U;
		break;
	case 0x4F:
		return O;
		break;
	case VK_SHIFT:
		return SHIFT;
		break;
	case VK_CONTROL:
		return CTRL;
		break;
	case VK_MENU:
		return ALT;
		break;
	case VK_F4:
		return F4;
		break;
	case 0x30:
		return ZERO;
		break;
	case 0x31:
		return ONE;
		break;
	case 0x32:
		return TWO;
		break;
	case 0x33:
		return THREE;
		break;
	case 0x34:
		return FOUR;
		break;
	case 0x35:
		return FIVE;
		break;
	case 0x36:
		return SIX;
		break;
	case 0x37:
		return SEVEN;
		break;
	case 0x38:
		return EIGHT;
		break;
	case 0x39:
		return NINE;
		break;

	default:
		return INVALID;
		break;
	}
}

static bool Win32ProcessMessages(win32_window &window, engine_input &input, i32 &quitMessage)
{
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		quitMessage = (i32)message.wParam;

		switch (message.message)
		{
		case WM_QUIT:
			return false;
			break;

		/***************** KEYBOARD EVENTS ****************/
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			bool wasDown = ((message.lParam >> 30) & 1) != 0;
			if (!wasDown || input.keyboard.autoRepeatEnabled)
			{
				KEYBOARD_BUTTON key = Win32TranslateKeyInput((VK_CODE)message.wParam);
				if (key < KEYBOARD_BUTTON::COUNT)
					input.keyboard.ToggleKey(key);
			}
			if (input.keyboard.isPressed[F4] && input.keyboard.isPressed[ALT])
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
			if (key < KEYBOARD_BUTTON::COUNT)
				input.keyboard.ToggleKey(key);
			break;
		}
		/**************************************************/

		/****************** MOUSE EVENTS ******************/
		case WM_MOUSEMOVE:
		{
			POINTS p = MAKEPOINTS(message.lParam);
			p.y = window.dimensions.height - p.y;
			bool isInWindow =
				p.x >= 0 && p.x < window.dimensions.width &&
				p.y >= 0 && p.y < window.dimensions.height;
			if (isInWindow)
			{
				input.mouse.SetPos(p.x, p.y);
				if (!input.mouse.isInWindow) // if it wasn't in the window before
				{
					SetCapture(window.handle);
					input.mouse.isInWindow = true;
				}
			}
			else
			{
				if (input.mouse.leftIsPressed || input.mouse.rightIsPressed)
				{
					// mouse is of the window but we're holding a button
					input.mouse.SetPos(p.x, p.y);
				}
				else
				{
					// mouse is out of the window
					ReleaseCapture();
					input.mouse.isInWindow = false;
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
			input.mouse.SetWheelDelta(input.mouse.WheelDelta() + GET_WHEEL_DELTA_WPARAM(message.wParam));
		case WM_MOUSELEAVE:
			POINTS p = MAKEPOINTS(message.lParam);
			input.mouse.SetPos(p.x, p.y);
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

static FILETIME Win32GetWriteTime(char *filename)
{
	FILETIME result = {};

	WIN32_FIND_DATA data;
	HANDLE handle = FindFirstFileA(filename, &data);
	if (handle != INVALID_HANDLE_VALUE)
	{
		result = data.ftLastWriteTime;
		FindClose(handle);
	}
	return result;
}

static void Win32LoadEngineCode(win32_engine_code *engineCode, char *sourceFilename, char *sourceFilenameCopy)
{
	// NOTE:  We need to add a sleep in order to wait for the dll compilation.
	Sleep(800);
	engineCode->writeTime = Win32GetWriteTime(sourceFilename);
	CopyFileA(sourceFilename, sourceFilenameCopy, FALSE);
	engineCode->dll = LoadLibraryA(sourceFilenameCopy);
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

static void Win32UnloadEngineCode(win32_engine_code *engineCode)
{
	if (engineCode->dll)
	{
		FreeLibrary(engineCode->dll);
		engineCode->dll = 0;
	}
	engineCode->isValid = false;
	engineCode->UpdateAndRender = UpdateAndRenderStub;
}

#if VULKAN_VALIDATION_LAYERS_ON
#include <iostream>
static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void *pUserData)
{
	bool isError = false;
	char *type;
	if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
		type = "Some general event has occurred";
	else if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
		type = "Something has occurred during validation against the Vulkan specification that may indicate invalid behavior.";
	else
		type = "Potentially non-optimal use of Vulkan.";

	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		std::cerr << "WARNING: " << type << "\n"
				  << pCallbackData->pMessageIdName << std::endl;
	else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		std::cerr << "DIAGNOSTIC: " << type << "\n"
				  << pCallbackData->pMessageIdName << std::endl;
	else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		std::cerr << "INFO: " << type << "\n"
				  << pCallbackData->pMessageIdName << std::endl;
	else
	{
		std::cerr << "ERROR: " << type << "\n"
				  << pCallbackData->pMessageIdName << std::endl;
		isError = true;
	}

	char c;
	u32 i = 0;
	do
	{
		c = pCallbackData->pMessage[i];
		i++;
	} while (c != ']');
	i++;
	do
	{
		c = pCallbackData->pMessage[i];
		if (c != ';' && c != '|')
		{
			std::cerr << c;
			i++;
		}
		else
		{
			std::cerr << std::endl;
			i++;
			do
			{
				c = pCallbackData->pMessage[i];
				i++;
			} while (c == ' ' || c == '|');
			i--;
		}
	} while (c != '\0');
	std::cerr << std::endl;
	if (isError)
		ASSERT(0);
	return VK_FALSE;
}
#endif

static bool Win32InitializeWindow(win32_window &window, i16 width, i16 height, LPCSTR windowTitle)
{
#if VULKAN_VALIDATION_LAYERS_ON
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout);
	freopen("CON", "w", stderr);
	SetConsoleTitle("Vulkan Debug");
#endif

	window.instance = GetModuleHandleW(nullptr);

	// Set window class properties
	WNDCLASSA windowClass = {};
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = Win32MainWindowProc;
	windowClass.lpszClassName = windowTitle;
	windowClass.hInstance = window.instance;
	windowClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	//windowClass.hIcon = LoadIconA(windowClass.hInstance, MAKEINTRESOURCEA(IDI_APPLICATION));

	if (!RegisterClassA(&windowClass))
	{
		OutputDebugStringA("ERROR: Window class wasn't registered.\n");
		return false;
	}

	window.dimensions.width = width;
	window.dimensions.height = height;
	// Declare the window client size
	RECT rect = {0};
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
		OutputDebugStringA("ERROR: Failed to create window.\n");
		return false;
	}

	// NOTE: A value to be passed to the window through the
	// CREATESTRUCT structure pointed to by the lParam of
	// the WM_CREATE message. This message is sent to the
	// created window by this function before it returns.

	ShowWindow(window.handle, SW_SHOWDEFAULT);
	return true;
}

static bool Win32InitializeVulkan(win32_window &window, wnd_dim dimensions)
{
	VkResult result;

#if VULKAN_VALIDATION_LAYERS_ON
	// NOTE: Enable Validation Layer
	const bool enableValidationLayers = true;

	char *validationLayers[] = {
		"VK_LAYER_KHRONOS_validation"};

	u32 layerCount;
	result = vkEnumerateInstanceLayerProperties(&layerCount, 0);
	ASSERT(result == VK_SUCCESS);

	VkLayerProperties availableLayers[16];
	ASSERT(layerCount <= ArrayCount(availableLayers));
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

	for (const char *layerName : validationLayers)
	{
		bool layerFound = false;
		for (const auto &layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}
		if (!layerFound)
		{
			return false;
		}
	}

	VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo = {};
	debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
										 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	//VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
	//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
	debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
									 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
									 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugMessengerInfo.pfnUserCallback = VulkanDebugCallback;

	char *instanceExtensions[] = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
#else
	const bool enableValidationLayers = false;

	char *instanceExtensions[] = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
#endif

	// NOTE: Set application info (optionl but usefule)
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = window.name;
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = window.name;
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// NOTE: Create an instance and attach extensions
	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledExtensionCount = ArrayCount(instanceExtensions);
	instanceInfo.ppEnabledExtensionNames = instanceExtensions;
#if VULKAN_VALIDATION_LAYERS_ON
	instanceInfo.enabledLayerCount = ArrayCount(validationLayers);
	instanceInfo.ppEnabledLayerNames = validationLayers;
	instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugMessengerInfo;
#endif

	result = vkCreateInstance(&instanceInfo, NULL, &window.vulkan.instance);
	ASSERT(result == VK_SUCCESS);

#if VULKAN_VALIDATION_LAYERS_ON
	PFN_vkCreateDebugUtilsMessengerEXT createDebugMessengerFunc =
		(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(window.vulkan.instance, "vkCreateDebugUtilsMessengerEXT");
	ASSERT(createDebugMessengerFunc != nullptr)
	{
		result = createDebugMessengerFunc(window.vulkan.instance, &debugMessengerInfo, 0, &window.vulkan.debugMessenger);
		ASSERT(result == VK_SUCCESS);
	}
#endif

	// NOTE: Select a gpu to use
	u32 gpuCount = 0;
	result = vkEnumeratePhysicalDevices(window.vulkan.instance, &gpuCount, 0);
	VkPhysicalDevice gpuBuffer[16] = {};
	ASSERT(gpuCount > 0 && gpuCount <= ArrayCount(gpuBuffer));
	result = vkEnumeratePhysicalDevices(window.vulkan.instance, &gpuCount, gpuBuffer);
	ASSERT(result == VK_SUCCESS);
	window.vulkan.gpu = gpuBuffer[0];
	// TODO: ACTUALY CHECK WHICH GPU IS BEST TO USE BY CHECKING THEIR QUEUES
	//For now it's ok since I only have 1 gpu.

	// NOTE: Get the queue families available for the gpu we selected
	u32 qFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(window.vulkan.gpu, &qFamilyCount, 0);

	VkQueueFamilyProperties qFamilyBuffer[16] = {};
	ASSERT(qFamilyCount <= ArrayCount(qFamilyBuffer));
	vkGetPhysicalDeviceQueueFamilyProperties(window.vulkan.gpu, &qFamilyCount, qFamilyBuffer);

	// NOTE: Create a surface
	VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.hinstance = window.instance;
	surfaceInfo.hwnd = window.handle;
	result = vkCreateWin32SurfaceKHR(window.vulkan.instance, &surfaceInfo, 0, &window.vulkan.surface);
	ASSERT(result == VK_SUCCESS);

	// NOTE: Find which queue family supports present and graphics operations
	VkBool32 supportsPresent[16] = {};
	for (u32 i = 0; i < qFamilyCount; i++)
	{
		result = vkGetPhysicalDeviceSurfaceSupportKHR(window.vulkan.gpu, i, window.vulkan.surface, &supportsPresent[i]);
		ASSERT(result == VK_SUCCESS);
	}

	window.vulkan.graphicsQueueFamilyIndex = UINT32_MAX;
	window.vulkan.presentQueueFamilyIndex = UINT32_MAX;
	for (u32 i = 0; i < qFamilyCount; ++i)
	{
		if ((qFamilyBuffer[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			if (window.vulkan.graphicsQueueFamilyIndex == UINT32_MAX)
				window.vulkan.graphicsQueueFamilyIndex = i;

			if (supportsPresent[i] == VK_TRUE)
			{
				window.vulkan.graphicsQueueFamilyIndex = i;
				window.vulkan.presentQueueFamilyIndex = i;
				break;
			}
		}
	}

	if (window.vulkan.presentQueueFamilyIndex == UINT32_MAX)
	{
		// If didn't find a queue that supports both graphics and present, then
		// find a separate present queue.
		for (u32 i = 0; i < qFamilyCount; ++i)
			if (supportsPresent[i] == VK_TRUE)
			{
				window.vulkan.presentQueueFamilyIndex = i;
				break;
			}
	}

	if (window.vulkan.graphicsQueueFamilyIndex == UINT32_MAX)
	{
		OutputDebugStringA("ERROR: No graphics queue found.\n");
		return false;
	}
	if (window.vulkan.presentQueueFamilyIndex == UINT32_MAX)
	{
		OutputDebugStringA("ERROR: No present queue found.\n");
		return false;
	}

	// NOTE: Create a device
	VkDeviceQueueCreateInfo queueInfo = {};
	float queuePriorities[1] = {0.0}; // must be array of size queueCount
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.queueFamilyIndex = window.vulkan.graphicsQueueFamilyIndex;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = queuePriorities;

	char *deviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.enabledExtensionCount = ArrayCount(deviceExtensions);
	deviceInfo.ppEnabledExtensionNames = deviceExtensions;

	result = vkCreateDevice(window.vulkan.gpu, &deviceInfo, 0, &window.vulkan.device);
	ASSERT(result == VK_SUCCESS);

	// NOTE: Create a command buffer
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = queueInfo.queueFamilyIndex;
	result = vkCreateCommandPool(window.vulkan.device, &cmdPoolInfo, 0, &window.vulkan.cmdPool);
	if (result != VK_SUCCESS)
	{
		OutputDebugStringA("ERROR: Failed to create vulkan command pool.\n");
		return false;
	}

	VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.commandPool = window.vulkan.cmdPool;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandBufferCount = 1;

	result = vkAllocateCommandBuffers(window.vulkan.device, &cmdBufferAllocInfo, &window.vulkan.cmdBuffer);
	ASSERT(result == VK_SUCCESS);

	// NOTE: Create a swapchain
	// NOTE: 1.set a proper surface format
	window.vulkan.surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	window.vulkan.surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;

	u32 formatCount = 0;
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(window.vulkan.gpu, window.vulkan.surface, &formatCount, nullptr);
	ASSERT(result == VK_SUCCESS);

	VkSurfaceFormatKHR availableFormats[16] = {};
	ASSERT(formatCount <= ArrayCount(availableFormats));
	for (u32 i = 0; i < formatCount; ++i)
	{
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(window.vulkan.gpu, window.vulkan.surface, &formatCount, &availableFormats[i]);
		ASSERT(result == VK_SUCCESS);
	}

	bool desiredSurfaceFormatSupported = false;
	for (u32 i = 0; i < formatCount; ++i)
	{
		if (window.vulkan.surfaceFormat.format == availableFormats[i].format &&
			window.vulkan.surfaceFormat.colorSpace == availableFormats[i].colorSpace)
		{
			desiredSurfaceFormatSupported = true;
		}
	}
	ASSERT(desiredSurfaceFormatSupported);

	// NOTE: Get Surface capabilities
	VkSurfaceCapabilitiesKHR surfCapabilities = {};
	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(window.vulkan.gpu, window.vulkan.surface, &surfCapabilities);
	ASSERT(result == VK_SUCCESS);

	VkExtent2D swapchainExtent = {};
	if (surfCapabilities.currentExtent.width == UINT32_MAX)
	{
		swapchainExtent.width = dimensions.width;
		swapchainExtent.height = dimensions.height;
		if (swapchainExtent.width < surfCapabilities.minImageExtent.width)
		{
			swapchainExtent.width = surfCapabilities.minImageExtent.width;
		}
		else if (swapchainExtent.width > surfCapabilities.maxImageExtent.width)
		{
			swapchainExtent.width = surfCapabilities.maxImageExtent.width;
		}

		if (swapchainExtent.height < surfCapabilities.minImageExtent.height)
		{
			swapchainExtent.height = surfCapabilities.minImageExtent.height;
		}
		else if (swapchainExtent.height > surfCapabilities.maxImageExtent.height)
		{
			swapchainExtent.height = surfCapabilities.maxImageExtent.height;
		}
	}
	else
	{
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfCapabilities.currentExtent;
	}

	// NOTE: Determine the number of VkImage's to use in the swap chain.
	// We need to acquire only 1 presentable image at at time.
	// Asking for minImageCount images ensures that we can acquire
	// 1 presentable image as long as we present it before attempting
	// to acquire another.
	uint32_t desiredNumberOfSwapChainImages = 2;
	if (surfCapabilities.maxImageCount < desiredNumberOfSwapChainImages)
	{
		desiredNumberOfSwapChainImages = surfCapabilities.minImageCount;
	}

	// NOTE: Determine the pre-transform
	VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; //do nothing
	if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surfCapabilities.currentTransform;
	}

	// NOTE: Set the present mode
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; //The FIFO present mode is guaranteed by the spec to be supported
															 /*
	uint32_t presentModeCount;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(window.vulkan.gpu, window.vulkan.surface, &presentModeCount, NULL);
	ASSERT(result == VK_SUCCESS);
	VkPresentModeKHR presentModes[16];
	ASSERT(presentModeCount <= ArrayCount(presentModes));
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(window.vulkan.gpu, window.vulkan.surface, &presentModeCount, presentModes);
	ASSERT(result == VK_SUCCESS);
	bool desiredPresentModeSupported = false;
	for (u32 i = 0; i < presentModeCount; i++)
	{
		if (presentMode == presentModes[i])
		{
			desiredPresentModeSupported = true;
		}
	}
	ASSERT(desiredPresentModeSupported);
	*/

	// NOTE: Find a supported composite alpha mode - one of these is guaranteed to be set
	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};
	for (uint32_t i = 0; i < ArrayCount(compositeAlphaFlags); i++)
	{
		if (surfCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i])
		{
			compositeAlpha = compositeAlphaFlags[i];
			break;
		}
	}

	VkSwapchainCreateInfoKHR swapchainInfo = {};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = window.vulkan.surface;
	swapchainInfo.minImageCount = desiredNumberOfSwapChainImages;
	swapchainInfo.imageFormat = window.vulkan.surfaceFormat.format;
	swapchainInfo.imageExtent = swapchainExtent;
	swapchainInfo.preTransform = preTransform;
	swapchainInfo.compositeAlpha = compositeAlpha;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.presentMode = presentMode;
	swapchainInfo.clipped = true;
	swapchainInfo.imageColorSpace = window.vulkan.surfaceFormat.colorSpace;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	u32 queueFamilyIndices[2] = {window.vulkan.graphicsQueueFamilyIndex, window.vulkan.presentQueueFamilyIndex};
	if (window.vulkan.graphicsQueueFamilyIndex != window.vulkan.presentQueueFamilyIndex)
	{
		// If the graphics and present queues are from different queue families,
		// we either have to explicitly transfer ownership of images between
		// the queues, or we have to create the swapchain with imageSharingMode
		// as VK_SHARING_MODE_CONCURRENT
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainInfo.queueFamilyIndexCount = 2;
		swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	result = vkCreateSwapchainKHR(window.vulkan.device, &swapchainInfo, 0, &window.vulkan.swapchain);
	ASSERT(result == VK_SUCCESS);

	result = vkGetSwapchainImagesKHR(window.vulkan.device, window.vulkan.swapchain, &window.vulkan.swapchainImageCount, 0);
	ASSERT(result == VK_SUCCESS);

	ASSERT(window.vulkan.swapchainImageCount == ArrayCount(window.vulkan.swapchainImages));

	result = vkGetSwapchainImagesKHR(window.vulkan.device, window.vulkan.swapchain, &window.vulkan.swapchainImageCount, window.vulkan.swapchainImages);
	ASSERT(result == VK_SUCCESS);

	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = window.vulkan.surfaceFormat.format;
	imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;
	for (u32 i = 0; i < window.vulkan.swapchainImageCount; i++)
	{
		imageViewInfo.image = window.vulkan.swapchainImages[i];
		result = vkCreateImageView(window.vulkan.device, &imageViewInfo, 0, &window.vulkan.imageViews[i]);
		ASSERT(result == VK_SUCCESS);
	}

	// NOTE: Create a depth buffer
	VkImageCreateInfo imageInfo = {};
	VkFormat depth_format = VK_FORMAT_D16_UNORM;
	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(window.vulkan.gpu, depth_format, &props);
	if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
	}
	else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	}
	else
	{
		ASSERT(depth_format != VK_FORMAT_D16_UNORM)
		return false;
	}
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = VK_FORMAT_D16_UNORM;
	imageInfo.extent.width = dimensions.width;
	imageInfo.extent.height = dimensions.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = NUM_SAMPLES;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	vkCreateImage(window.vulkan.device, &imageInfo, 0, &window.vulkan.depthImage);

	return true;
}

static bool Win32InitializeMemory(engine_memory &memory)
{
	LPVOID baseAddress = (LPVOID)TERABYTES(2);
	memory.permanentMemorySize = MEGABYTES(64);
	memory.transientMemorySize = GIGABYTES(2);
	u64 totalSize = memory.permanentMemorySize + memory.transientMemorySize;
	memory.permanentMemory = VirtualAlloc(baseAddress, totalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	memory.transientMemory = (u8 *)memory.permanentMemory + memory.permanentMemorySize;
	memory.isInitialized = false;

	memory.DEBUGFreeFileMemory = DEBUGFreeFileMemory;
	memory.DEBUGReadFile = DEBUGReadFile;
	memory.DEBUGWriteFile = DEBUGWriteFile;

	return (memory.permanentMemory && memory.transientMemory);
}

int CALLBACK WinMain(
	HINSTANCE instance,
	HINSTANCE prevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	wnd_dim dim = {};
	dim.width = 512;
	dim.height = 512;
	win32_window window;
	if (!Win32InitializeWindow(window, dim.width, dim.height, window.name))
	{
		return 1;
	}
	OutputDebugString("Test");

	if (!Win32InitializeVulkan(window, dim))
	{
		return 2;
	}

	engine_memory engineMemory;
	if (!Win32InitializeMemory(engineMemory))
	{
		return 3;
	}

	win32_engine_code engineCode = {};

	// TODO: make this less explicit
	char *sourceDLLPath = "W:\\heyoVulkan\\build\\hyv_engine.dll";
	char *sourceDLLCopyPath = "W:\\heyoVulkan\\build\\hyv_engine_copy.dll";
	Win32LoadEngineCode(&engineCode, sourceDLLPath, sourceDLLCopyPath);

	hyv_engine engine = {};
	i32 quitMessage = -1;
	while (Win32ProcessMessages(window, engine.input, quitMessage))
	{
		FILETIME newWriteTime = Win32GetWriteTime(sourceDLLPath);
		if (CompareFileTime(&newWriteTime, &engineCode.writeTime) == 1)
		{
			Win32UnloadEngineCode(&engineCode);
			Win32LoadEngineCode(&engineCode, sourceDLLPath, sourceDLLCopyPath);
		}
		engineCode.UpdateAndRender(engine, &engineMemory);
		Win32Update(window);
	}
	return quitMessage;
}