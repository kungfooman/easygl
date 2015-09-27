/***********************************************
*                                              *
*        Jeff Molofee's OpenGL Basecode        *
*                                              *
*            http://nehe.gamedev.net           *
*                                              *
*                    2003                      *
*                                              *
***********************************************/

#include <windows.h>													// Header File For The Windows Library
#include <gl/gl.h>														// Header File For The OpenGL32 Library
#include <gl/glu.h>														// Header File For The GLu32 Library
#include <new.h>
#include "NeHeGL.h"														// Header File For The NeHeGL Basecode
#include <stdio.h>

#define WM_TOGGLEFULLSCREEN (WM_USER+1)									// Application Define Message For Toggling

void WindowTerminate (gl_window_t *win)							// Terminate The Application
{
	PostMessage (win->hWnd, WM_QUIT, 0, 0);							// Send A WM_QUIT Message
}

void ToggleFullscreen (gl_window_t *win)								// Toggle Fullscreen/Windowed
{
	PostMessage (win->hWnd, WM_TOGGLEFULLSCREEN, 0, 0);				// Send A WM_TOGGLEFULLSCREEN Message
}

void ReshapeGL (gl_window_t *win, int width, int height)									// Reshape The Window When It's Moved Or Resized
{
	glViewport (0, 0, (GLsizei)(width), (GLsizei)(height));				// Reset The Current Viewport
}

BOOL ChangeScreenResolution (int width, int height, int bitsPerPixel)	// Change The Screen Resolution
{
	DEVMODE dmScreenSettings;											// Device Mode
	ZeroMemory (&dmScreenSettings, sizeof (DEVMODE));					// Make Sure Memory Is Cleared
	dmScreenSettings.dmSize				= sizeof (DEVMODE);				// Size Of The Devmode Structure
	dmScreenSettings.dmPelsWidth		= width;						// Select Screen Width
	dmScreenSettings.dmPelsHeight		= height;						// Select Screen Height
	dmScreenSettings.dmBitsPerPel		= bitsPerPixel;					// Select Bits Per Pixel
	dmScreenSettings.dmFields			= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	if (ChangeDisplaySettings (&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		return FALSE;													// Display Change Failed, Return False
	}
	return TRUE;														// Display Change Was Successful, Return True
}

void CreateWindowGL (gl_window_t *win)									// This Code Creates Our OpenGL Window
{
	__int64	timer;														// Holds High Resolution Timer Information
	DWORD windowStyle = WS_OVERLAPPEDWINDOW;							// Define Our Window Style
	DWORD windowExtendedStyle = WS_EX_APPWINDOW;						// Define The Window's Extended Style

	PIXELFORMATDESCRIPTOR pfd =											// pfd Tells Windows How We Want Things To Be
	{
		sizeof (PIXELFORMATDESCRIPTOR),									// Size Of This Pixel Format Descriptor
		1,																// Version Number
		PFD_DRAW_TO_WINDOW |											// Format Must Support Window
		PFD_SUPPORT_OPENGL |											// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,												// Must Support Double Buffering
		PFD_TYPE_RGBA,													// Request An RGBA Format
		win->init.bitsPerPixel,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,												// Color Bits Ignored
		0,																// No Alpha Buffer
		0,																// Shift Bit Ignored
		0,																// No Accumulation Buffer
		0, 0, 0, 0,														// Accumulation Bits Ignored
		16,																// 16Bit Z-Buffer (Depth Buffer)  
		0,																// No Stencil Buffer
		0,																// No Auxiliary Buffer
		PFD_MAIN_PLANE,													// Main Drawing Layer
		0,																// Reserved
		0, 0, 0															// Layer Masks Ignored
	};

	RECT windowRect = {0, 0, win->init.width, win->init.height};	// Define Our Window Coordinates

	GLuint PixelFormat;													// Will Hold The Selected Pixel Format

	if (win->init.isFullScreen == TRUE)								// Fullscreen Requested, Try Changing Video Modes
	{
		if (ChangeScreenResolution (win->init.width, win->init.height, win->init.bitsPerPixel) == FALSE)
		{
			// Fullscreen Mode Failed.  Run In Windowed Mode Instead
			MessageBox (HWND_DESKTOP, "Mode Switch Failed.\nRunning In Windowed Mode.", "Error", MB_OK | MB_ICONEXCLAMATION);
			win->init.isFullScreen = FALSE;							// Set isFullscreen To False (Windowed Mode)
		}
		else															// Otherwise (If Fullscreen Mode Was Successful)
		{
			ShowCursor (FALSE);											// Turn Off The Cursor
			windowStyle = WS_POPUP;										// Set The WindowStyle To WS_POPUP (Popup Window)
			windowExtendedStyle |= WS_EX_TOPMOST;						// Set The Extended Window Style To WS_EX_TOPMOST
		}																// (Top Window Covering Everything Else)
	}
	else																// If Fullscreen Was Not Selected
	{
		// Adjust Window, Account For Window Borders
		AdjustWindowRectEx (&windowRect, windowStyle, 0, windowExtendedStyle);
	}

	// Create The OpenGL Window
	win->hWnd = CreateWindowExA (windowExtendedStyle,					// Extended Style
								   win->init.application->className,	// Class Name
								   win->init.title,					// Window Title
								   windowStyle,							// Window Style
								   win->init.left, win->init.top,								// Window X,Y Position
								   windowRect.right - windowRect.left,	// Window Width
								   windowRect.bottom - windowRect.top,	// Window Height
								   HWND_DESKTOP,						// Desktop Is Window's Parent
								   0,									// No Menu
								   win->init.application->hInstance, // Pass The Window Instance
								   win);

	if (win->hWnd == 0)												// Was Window Creation A Success?
	{
		win->isCreated = FALSE;
		return;
	}

	win->hDC = GetDC (win->hWnd);									// Grab A Device Context For This Window
	if (win->hDC == 0)												// Did We Get A Device Context?
	{
		// Failed
		DestroyWindow (win->hWnd);									// Destroy The Window
		win->hWnd = 0;												// Zero The Window Handle
		win->isCreated = FALSE;
		return;													// Return False
	}

	PixelFormat = ChoosePixelFormat (win->hDC, &pfd);				// Find A Compatible Pixel Format
	if (PixelFormat == 0)												// Did We Find A Compatible Format?
	{
		// Failed
		ReleaseDC (win->hWnd, win->hDC);							// Release Our Device Context
		win->hDC = 0;												// Zero The Device Context
		DestroyWindow (win->hWnd);									// Destroy The Window
		win->hWnd = 0;												// Zero The Window Handle
		win->isCreated = FALSE;
		return;													// Return False
	}

	if (SetPixelFormat (win->hDC, PixelFormat, &pfd) == FALSE)		// Try To Set The Pixel Format
	{
		// Failed
		ReleaseDC (win->hWnd, win->hDC);							// Release Our Device Context
		win->hDC = 0;												// Zero The Device Context
		DestroyWindow (win->hWnd);									// Destroy The Window
		win->hWnd = 0;												// Zero The Window Handle
		win->isCreated = FALSE;
		return;													// Return False
	}

	win->hRC = wglCreateContext (win->hDC);						// Try To Get A Rendering Context
	if (win->hRC == 0)												// Did We Get A Rendering Context?
	{
		// Failed
		ReleaseDC (win->hWnd, win->hDC);							// Release Our Device Context
		win->hDC = 0;												// Zero The Device Context
		DestroyWindow (win->hWnd);									// Destroy The Window
		win->hWnd = 0;												// Zero The Window Handle
		win->isCreated = FALSE;
		return;													// Return False
	}

	// Make The Rendering Context Our Current Rendering Context
	if (wglMakeCurrent (win->hDC, win->hRC) == FALSE)
	{
		// Failed
		wglDeleteContext (win->hRC);									// Delete The Rendering Context
		win->hRC = 0;												// Zero The Rendering Context
		ReleaseDC (win->hWnd, win->hDC);							// Release Our Device Context
		win->hDC = 0;												// Zero The Device Context
		DestroyWindow (win->hWnd);									// Destroy The Window
		win->hWnd = 0;												// Zero The Window Handle
		win->isCreated = FALSE;
		return;													// Return False
	}

	ShowWindow (win->hWnd, SW_NORMAL);								// Make The Window Visible
	win->isVisible = TRUE;											// Set isVisible To True

	ReshapeGL (win, win->init.width, win->init.height);				// Reshape Our GL Window

	ZeroMemory (&(win->keys), sizeof (keys_t));							// Clear All Keys

	if (QueryPerformanceFrequency((LARGE_INTEGER *) &timer))			// Check To See If We Can Use Peformance Counter
	{
		win->hrTimer=TRUE;											// High Resolution Is Available
		// Grab The Starting Tick Value
		win->lastTickCount = QueryPerformanceCounter((LARGE_INTEGER *) &timer);
		// Grab The Counter Frequency
		QueryPerformanceFrequency((LARGE_INTEGER *) &timer);
		// Set The Timer Resolution 1.0f / Timer Frequency
		win->timerResolution = (float) (((double)1.0f)/((double)timer));
	} else {
		win->lastTickCount = GetTickCount ();						// Get Tick Count The Old Fashioned Way
		win->timerResolution = 1.0f/1000.0f;							// Set Our Timer Resolution To .001f
	}
	win->isCreated = TRUE;
	return;
																		// Initialization Will Be Done In WM_CREATE
}

BOOL DestroyWindowGL (gl_window_t *win)								// Destroy The OpenGL Window & Release Resources
{
	if (win->hWnd != 0)												// Does The Window Have A Handle?
	{	
		if (win->hDC != 0)											// Does The Window Have A Device Context?
		{
			wglMakeCurrent (win->hDC, 0);							// Set The Current Active Rendering Context To Zero
			if (win->hRC != 0)										// Does The Window Have A Rendering Context?
			{
				wglDeleteContext (win->hRC);							// Release The Rendering Context
				win->hRC = 0;										// Zero The Rendering Context
			}
			ReleaseDC (win->hWnd, win->hDC);						// Release The Device Context
			win->hDC = 0;											// Zero The Device Context
		}
		DestroyWindow (win->hWnd);									// Destroy The Window
		win->hWnd = 0;												// Zero The Window Handle
	}

	if (win->init.isFullScreen)										// Is Window In Fullscreen Mode
	{
		ChangeDisplaySettings (NULL,0);									// Switch Back To Desktop Resolution
		ShowCursor (TRUE);												// Show The Cursor
	}	
	return TRUE;														// Return True
}

void WindowDraw(gl_window_t *win) {
	//wglSwapInter
	WindowMakeCurrent(win);
	win->callbackDraw(win);
	SwapBuffers(win->hDC);
	//glFinish(); // may cause fps drop or increase lol https://www.opengl.org/wiki/Swap_Interval
}

void WindowUpdate(gl_window_t *win) {
	DWORD		tickCount;												// Holds Current Tick Count
	__int64		timer;													// Used For The Tick Counter
	if (win->hrTimer) {										// Are We Using High Resolution Timer
	QueryPerformanceCounter((LARGE_INTEGER *) &timer);		// Grab Current Value In Performance Counter
	tickCount = (DWORD) timer;								// Store It In Tick Count
	}
	else														// Otherwise We Grab Ticks The Old Fashioned Way
	tickCount = GetTickCount ();							// Get The Tick Count
	// Call Update With The New Counter Value
	win->deltaTime = (tickCount - win->lastTickCount) * win->timerResolution * 1000.0f;
	win->lastTickCount = tickCount;							// Set Last Count To Current Count

	win->callbackUpdate(win);
}



// Process Window Message Callbacks
LRESULT CALLBACK WindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	// Get The Window Context
	gl_window_t *win = (gl_window_t *)(GetWindowLong (hWnd, GWL_USERDATA));
	//printf("hwnd=%d uMsg=%d\n", hWnd, uMsg);
	switch (uMsg) {
		case WM_ERASEBKGND:												// Check To See If Windows Is Trying To Erase The Background
			return 0;													// Return 0 (Prevents Flickering While Resizing A Window)

		case WM_PAINT:													// Window Needs Updating
		{
			//PAINTSTRUCT ps;
			//BeginPaint(win->hWnd, &ps);

			// uncomment this to repaint window on size/move event, maybe add win->sizemoveevent = TRUE,
			// so it will not interfer with main loop

			if (win->win32_wndproconly) {
				WindowUpdate(win);
				WindowDraw(win);
			}

			//  I've read that opengl should be drawn in WM_PAINT, but that just caused problems (unfocused window e.g. just would not update at all)
			//EndPaint(win->hWnd, &ps);
			return 0;
		}

		case WM_SYSCOMMAND:												// Intercept System Commands
		{
			switch (wParam)												// Check System Calls
			{
				case SC_SCREENSAVE:										// Screensaver Trying To Start?
				case SC_MONITORPOWER:									// Monitor Trying To Enter Powersave?
				return 0;												// Prevent From Happening
			}
			break;														// Call DefWindowProc()
		}


		case WM_CREATE:													// Window Creation
		{
			CREATESTRUCT* creation = (CREATESTRUCT*)(lParam);			// Store Window Structure Pointer
			win = (gl_window_t *)(creation->lpCreateParams);
			SetWindowLong (hWnd, GWL_USERDATA, (LONG)(win));
			return 0;
		}


		case WM_CLOSE:
			WindowTerminate(win);
			return 0;

		case WM_SIZE:													// Size Action Has Taken Place
			//printf("WM_SIZE\n");
			switch (wParam) {
				case SIZE_MINIMIZED:									// Was Window Minimized?
					win->isVisible = FALSE;							// Set isVisible To False
					return 0;

				case SIZE_MAXIMIZED:									// Was Window Maximized?
					win->isVisible = TRUE;							// Set isVisible To True
					WindowMakeCurrent(win);
					ReshapeGL (win, LOWORD (lParam), HIWORD (lParam));		// Reshape Window - LoWord=Width, HiWord=Height
					return 0;

				case SIZE_RESTORED:										// Was Window Restored?
					win->isVisible = TRUE;							// Set isVisible To True
					WindowMakeCurrent(win);
					ReshapeGL (win, LOWORD (lParam), HIWORD (lParam));		// Reshape Window - LoWord=Width, HiWord=Height
					return 0;
			}
			break;

		case WM_KEYDOWN:												// Update Keyboard Buffers For Keys Pressed
			if ((wParam >= 0) && (wParam <= 255))						// Is Key (wParam) In A Valid Range?
			{
				win->keys.keyDown [wParam] = TRUE;					// Set The Selected Key (wParam) To True
				return 0;												// Return
			}
			break;

		case WM_KEYUP:													// Update Keyboard Buffers For Keys Released
			if ((wParam >= 0) && (wParam <= 255))						// Is Key (wParam) In A Valid Range?
			{
				win->keys.keyDown [wParam] = FALSE;					// Set The Selected Key (wParam) To False
				return 0;												// Return
			}
			break;

		case WM_TOGGLEFULLSCREEN:										// Toggle FullScreen Mode On/Off
			//win->toggleFullscreen = TRUE;
			PostMessage (hWnd, WM_QUIT, 0, 0);
			break;
	}

	return DefWindowProc (hWnd, uMsg, wParam, lParam);					// Pass Unhandled Messages To DefWindowProc
}

BOOL RegisterWindowClass (application_t *application)						// Register A Window Class For This Application.
{																		// TRUE If Successful
	// Register A Window Class
	WNDCLASSEX windowClass;												// Window Class
	ZeroMemory (&windowClass, sizeof (WNDCLASSEX));						// Make Sure Memory Is Cleared
	windowClass.cbSize			= sizeof (WNDCLASSEX);					// Size Of The windowClass Structure

	// tldr: WinAPI calls WM_PAINT directly with WM_PAINT on resize/move event, main loop still blocks
	// http://stackoverflow.com/questions/3102074/win32-my-application-freezes-while-the-user-resizes-the-window/32806642#32806642
	windowClass.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraws The Window For Any Movement / Resizing
	windowClass.lpfnWndProc		= (WNDPROC)(WindowProc);				// WindowProc Handles Messages
	windowClass.hInstance		= application->hInstance;				// Set The Instance
	windowClass.hbrBackground	= (HBRUSH)COLOR_WINDOW+1;				// Class Background Brush Color
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	windowClass.lpszClassName	= application->className;				// Sets The Applications Classname
	if (RegisterClassEx (&windowClass) == 0)							// Did Registering The Class Fail?
	{
		// NOTE: Failure, Should Never Happen
		MessageBox (HWND_DESKTOP, "RegisterClassEx Failed!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;													// Return False (Failure)
	}
	return TRUE;														// Return True (Success)
}

void WindowMessageLoop(gl_window_t *win) {
	BOOL isMessagePumpActive;
	MSG msg;
	if (PeekMessageA (&msg, win->hWnd, 0, 0, PM_REMOVE) != 0) {
		//printf("WindowMessageLoop\n");
		if (msg.message != WM_QUIT) {
			TranslateMessage(&msg); // needed for what? NeHe didn't call this
			DispatchMessageA (&msg);
		} else {
			WindowClose(win);
		}
	} else {
		if ( ! win->isVisible) {
			//WaitMessage();
		}
	}
}

gl_window_t *WindowNew(
	application_t *application,
	char *title,
	int left, int top, int width, int height,
	window_callback_t initialize,
	window_callback_t deinitialize,
	window_callback_t update,
	window_callback_t draw
) {
	gl_window_t *win = (gl_window_t *) malloc(sizeof(gl_window_t));
	ZeroMemory (win, sizeof (gl_window_t));
	win->init.title				= title;
	win->init.left				= left;
	win->init.top				= top;
	win->init.width				= width;
	win->init.height				= height;
	win->init.bitsPerPixel		= 32;
	win->init.isFullScreen		= FALSE;
	win->callbackInitialize		= initialize;
	win->callbackDeinitialize	= deinitialize;
	win->callbackUpdate			= update;
	win->callbackDraw			= draw;
	win->init.application = application; // needed in CreateWindowGL
	CreateWindowGL(win);
	return win;
}

application_t *ApplicationNew(char *classname, HINSTANCE tmp) {
	application_t *application = (application_t *) malloc(sizeof(application_t));
	application->className = classname;
	application->hInstance = tmp;
	//application->hInstance = hInstance;

	// Register A Class For Our Window To Use
	if (RegisterWindowClass(application) == FALSE)					// Did Registering A Class Fail?
	{
		// Failure
		MessageBox (HWND_DESKTOP, "Error Registering Window Class!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return NULL;														// Terminate Application
	}

	return application;
}

void ApplicationClose(application_t *application) {
	UnregisterClass (application->className, application->hInstance);		// UnRegister Window Class
}

void WindowClose(gl_window_t *win) {
	if ( ! win->isCreated)
		return;
	win->callbackDeinitialize(win); // User Defined DeInitialization
	DestroyWindowGL(win);

	// Well, function succeeds, but not needed?!
	//BOOL ret = UnregisterClassA(win->init.application->className, win->init.application->hInstance);
	//printf("UnregisterClass=%d\n", ret);
	win->isCreated = FALSE;
	win->quit = TRUE;
}

void WindowMakeCurrent(gl_window_t *win) {
	wglMakeCurrent (win->hDC, win->hRC);
	if (wglMakeCurrent (win->hDC, win->hRC) == FALSE) {
		//wglDeleteContext (win->hRC);									// Delete The Rendering Context
		//win->hRC = 0;												// Zero The Rendering Context
		//ReleaseDC (win->hWnd, win->hDC);							// Release Our Device Context
		//win->hDC = 0;												// Zero The Device Context
		//DestroyWindow (win->hWnd);									// Destroy The Window
		//win->hWnd = 0;												// Zero The Window Handle
		//win->error = "WindowMakeCurrent: wglMakeCurrent(...) failed";
	}
}

void WindowInit(gl_window_t *win) {
	WindowMakeCurrent(win);
	win->callbackInitialize(win);
}