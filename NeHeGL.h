/********************
*                   *
*   NeHeGL Header   *
*                   *
**********************************************************************************
*                                                                                *
*	You Need To Provide The Following Functions:                                 *
*                                                                                *
*	void Initialize (GL_Window* window);                             *
*		Performs All Your Initialization                                         *
*		Returns TRUE If Initialization Was Successful, FALSE If Not              *
*		'window' Is A Parameter Used In Calls To NeHeGL                          *
*		'keys' Is A Structure Containing The Up/Down Status Of keys              *
*                                                                                *
*	void Deinitialize (void);                                                    *
*		Performs All Your DeInitialization                                       *
*                                                                                *
*	void Update (float milliseconds);                                            *
*		Perform Motion Updates                                                   *
*		'milliseconds' Is The Number Of Milliseconds Passed Since The Last Call  *
*		With Whatever Accuracy GetTickCount() Provides When A High Resolution    *
*       Timer Is Not Available                                                   *
*                                                                                *
*	void Draw (void);                                                            *
*		Perform All Your Scene Drawing                                           *
*                                                                                *
*********************************************************************************/

#ifndef GL_FRAMEWORK__INCLUDED
#define GL_FRAMEWORK__INCLUDED

#ifndef CDS_FULLSCREEN								// CDS_FULLSCREEN Is Not Defined By Some
#define CDS_FULLSCREEN 4							// Compilers. By Defining It This Way,
#endif												// We Can Avoid Errors

#include <windows.h>								// Header File For Windows

typedef struct keys_s {								// Structure For Keyboard Stuff
	BOOL keyDown [256];								// Holds TRUE / FALSE For Each Key
} keys_t;											// Keys

typedef struct application_s {						// Contains Information Vital To Applications
	HINSTANCE		hInstance;						// Application Instance
	const char*		className;						// Application ClassName
} application_t;									// Application

typedef struct gl_windowinit_s {					// Window Creation Info
	application_t		*application;				// Application Structure
	char*				title;						// Window Title
	int					left;
	int					top;
	int					width;						// Width
	int					height;						// Height
	int					bitsPerPixel;				// Bits Per Pixel
	BOOL				isFullScreen;				// FullScreen?
} gl_windowinit_t;									// GL_WindowInit

//typedef void (*window_callback_t)(gl_window_t *window);
struct window_callback_s;

typedef void (*window_callback_t)(struct gl_window_s *window);

typedef struct gl_window_s {						// Contains Information Vital To A Window
	keys_t				keys;						// Key Structure
	HWND				hWnd;						// Window Handle
	HDC					hDC;						// Device Context
	HGLRC				hRC;						// Rendering Context
	gl_windowinit_t		init;						// Window Init
	BOOL				isVisible;					// Window Visible?
	BOOL				hrTimer;					// High Resolution Timer Available?
	DWORD				lastTickCount;				// Tick Counter
	float				timerResolution;			// Timer Resolution
	int					isInitialized;
	float				deltaTime;
	window_callback_t	callbackInitialize;
	window_callback_t	callbackDeinitialize;
	window_callback_t	callbackUpdate;
	window_callback_t	callbackDraw;
	BOOL quit;
	BOOL isCreated;
	void *userdata;
	BOOL win32_wndproconly;
} gl_window_t;										// GL_Window




void WindowTerminate (gl_window_t *window);		// Terminate The Application
void ToggleFullscreen (gl_window_t *window);			// Toggle Fullscreen / Windowed Mode

BOOL RegisterWindowClass (application_t *application);
void WindowMessageLoop(gl_window_t *window);
gl_window_t *WindowNew(
	application_t *application,
	char *title,
	int left, int top, int width, int height,
	window_callback_t initialize,
	window_callback_t deinitialize,
	window_callback_t update,
	window_callback_t draw
);
void CreateWindowGL (gl_window_t *window);
BOOL DestroyWindowGL (gl_window_t *window);

application_t *ApplicationNew(char *classname, HINSTANCE tmp);
void ApplicationClose(application_t *application);
void WindowClose(gl_window_t *window);
void WindowMakeCurrent(gl_window_t *win);
void WindowDraw(gl_window_t *win);
void WindowUpdate(gl_window_t *win);
void WindowInit(gl_window_t *win);

#endif												// GL_FRAMEWORK__INCLUDED
