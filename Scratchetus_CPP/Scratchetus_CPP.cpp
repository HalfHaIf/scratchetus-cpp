#include "stdafx.h"

#include "Scratchetus_CPP.h"

#include "fileexists.h"

#include "draw.h"

#include "font.h" 
//TODO: Get these includes into stdafx.h
#include <sstream>

#include <string>

#include <commdlg.h>

#include <vector>

#include <windowsx.h>

#include <Shlwapi.h>

#define LIGHT_MODE 1
#define DARK_MODE 2
#define NO_BUILD_SELECTED -1

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

/// struct to hold build name and path
struct BuildInfo
{
    std::wstring name;
    std::wstring path;
};


// In VS2008, we can't define variables in WndProc

HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

HMENU hMenuBar;

// Forward declarations of functions included in this code module
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

//HWND that stores the button to launch Squeak
HWND				leftPanel;
HWND				rightPanel;
HWND				LaunchSqueakButton;

//Rectangle containing the window dimensions
RECT                windowrect;

// "Add Build" dialog stuff
wchar_t* filename;

wchar_t szFile[260];

OPENFILENAME ofn;



// Buffer
BuildInfo temp;

//Vector containing the names and file paths of all installed builds
std::vector<BuildInfo> builds;

//Stores where the mouse clicked last
POINT mousecoords;

// Buffer mainly used for converting wstrings to LPCWSTR
LPCWSTR lpcwstr_buffer;
LPCWSTR lpcwstr_buffer2;

LPWSTR lpwstr_buffer;

std::wstring wstring_buffer;

std::wstringstream wss_buffer;

int int_buffer;

//Stores which build we've selected
short int selected_build = NO_BUILD_SELECTED;

//Stores which theme we're using
short int theme;

wchar_t SqueakPath[256];
bool SqueakPathDefined = false;

//Full path of the Scrape files, used for not writing scrape.ini to the wrong place
wchar_t exeFullPath[256];

HBRUSH				RightSideBrush;


bool DeleteMode = false;

LRESULT CALLBACK LeftWndProc(HWND hwnd_left, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool DeleteMode;

	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{

    case WM_LBUTTONDOWN:
		mousecoords.x = GET_X_LPARAM(lParam);
		mousecoords.y = GET_Y_LPARAM(lParam);

		for(size_t i = 0; i < builds.size(); ++i) 
		{
			RECT buildrect;
			buildrect.left = 0;
			buildrect.top = i * 32;
			buildrect.right = windowrect.right << 1; // Half the window's width
			buildrect.bottom = 32 + (i * 32); // Fixed height

			if(PtInRect(&buildrect, mousecoords))
			{	
				selected_build = i;
				lpcwstr_buffer = builds[i].name.c_str();				
				break;
			}
			else
			{
					selected_build = NO_BUILD_SELECTED;
			}
		}
		InvalidateRect(hwnd_left, NULL, TRUE); // need to redraw the window, selected build has probably changed
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		draw(hdc, hwnd_left, ps);
	default:
		return DefWindowProc(hwnd_left, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK RightWndProc(HWND hwnd_right, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool DeleteMode;

	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
			case ID_BUTTON:
				if (selected_build != NO_BUILD_SELECTED) {
					//Run squeak and pass the name of the currently selected build to it
					lpcwstr_buffer = builds[selected_build].path.c_str();
					//We have selected a build, let's open it!
					ShellExecute(hwnd_right, L"open", SqueakPath, lpcwstr_buffer, NULL, SW_SHOWDEFAULT);
					MessageBox(NULL, SqueakPath, L"exe", MB_OK | MB_ICONINFORMATION);
					MessageBox(NULL, lpcwstr_buffer, L"params", MB_OK | MB_ICONINFORMATION);
					int error = GetLastError();
					if (error == 2) {
						MessageBox(NULL, L"well shit.", L"Info", MB_OK | MB_ICONINFORMATION);
					}
					
				}
				else
				{
					//No build selected, which one are we supposed to open?
					MessageBox(NULL, L"No build selected! Select a build first.", L"Info", MB_OK | MB_ICONINFORMATION);
				}
				break;
		}

	case WM_CREATE:
		CreateWindow(L"BUTTON", L"Launch Squeak", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
        100, 250, 125, 30, hwnd_right, (HMENU)ID_BUTTON, NULL, NULL);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd_right, &ps);
		if (theme == DARK_MODE) {
			//dark mode
			RightSideBrush = CreateSolidBrush(RGB(51, 51, 67));	
			//set text to color to white in dark mode
			SetTextColor(hdc, RGB(255, 255, 255));
		}
		else
		{
			//light mode
			RightSideBrush = CreateSolidBrush(RGB(255, 255, 255));
		}
		FillRect(hdc, &windowrect, RightSideBrush);
		EndPaint(hwnd_right, &ps);
	default:
		return DefWindowProc(hwnd_right, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{


	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
			case IDM_ADDBUILD:
				//Open the dialog
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = szFile;
                ofn.lpstrFile[0] = L'\0';
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrFileTitle = NULL;
                ofn.nMaxFileTitle = 0;
                ofn.lpstrInitialDir = NULL;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                if(GetOpenFileName(&ofn) == TRUE)
                {
					temp.path = ofn.lpstrFile;
					
					//Copy to the LPWSTR buffer, the function to remove the filename only accepts LPWSTRs
					lpwstr_buffer = const_cast<wchar_t*>(temp.path.c_str());

					//Remove the filename of the build from the path element
					//Transfer the data back to the proper path variable
					temp.path = lpwstr_buffer;

					filename = wcsrchr(ofn.lpstrFile, L'\\');
					if(filename != NULL) {
						temp.name = filename + 1;
					}
					//Add to the builds vector
					builds.push_back(temp);
                }
				
				wss_buffer.str(L"");
				wss_buffer.clear();

				//Write how many builds have been added (will be used for reading the INI later)
				wss_buffer << builds.size();
				WritePrivateProfileString(L"General", L"numbuilds", wss_buffer.str().c_str(), exeFullPath);

				for (int i = 0; i < builds.size(); i++)
				{
					wss_buffer.str(L"");
					wss_buffer.clear();
					wss_buffer << L"Build" << i;
					std::wstring section = wss_buffer.str();
					WritePrivateProfileString(section.c_str(), L"name", builds[i].name.c_str(), exeFullPath);
					WritePrivateProfileString(section.c_str(), L"path", builds[i].path.c_str(), exeFullPath);
				}

				InvalidateRect(hwnd, NULL, TRUE);

				break;
			case IDM_REMOVEBUILD:
				builds.erase(builds.begin() + selected_build);
				//Rewrite builds list to INI

				wss_buffer.str(L"");
				wss_buffer.clear();

				//Write how many builds have been added (will be used for reading the INI later)
				wss_buffer << builds.size();
				WritePrivateProfileString(L"General", L"numbuilds", wss_buffer.str().c_str(), exeFullPath);
				for (int i = 0; i < builds.size(); i++)
				{
					wss_buffer.str(L"");
					wss_buffer.clear();
					wss_buffer << L"Build" << i;
					std::wstring section = wss_buffer.str();
					WritePrivateProfileString(section.c_str(), L"name", builds[i].name.c_str(), exeFullPath);
					WritePrivateProfileString(section.c_str(), L"path", builds[i].path.c_str(), exeFullPath);
				}
				InvalidateRect(hwnd, NULL, TRUE); 
				break;
			case IDM_ABOUT:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
				break;
			case IDM_EXIT:
				DestroyWindow(hwnd);
				break;
			case ID_THEME_LIGHTMODE:
				//Uncheck dark mode
				CheckMenuItem(hMenuBar, ID_THEME_DARKMODE, MF_BYCOMMAND | MF_UNCHECKED);
				//Check light mode
				CheckMenuItem(hMenuBar, ID_THEME_LIGHTMODE, MF_BYCOMMAND | MF_CHECKED);
				//Set theme to light mode
				theme = LIGHT_MODE;
				//Theme has probably changed, so:

				//Redraw window
				InvalidateRect(hwnd, NULL, TRUE); 
				//Save theme to scrape.ini
				wss_buffer.str(L"");
				wss_buffer.clear();
				wss_buffer << theme;
				WritePrivateProfileString(L"General", L"theme", wss_buffer.str().c_str(), exeFullPath);
				break;
			case ID_THEME_DARKMODE:
				//Uncheck light mode
				CheckMenuItem(hMenuBar, ID_THEME_LIGHTMODE, MF_BYCOMMAND | MF_UNCHECKED);
				//Check dark mode
				CheckMenuItem(hMenuBar, ID_THEME_DARKMODE, MF_BYCOMMAND | MF_CHECKED);
				//Set theme to light mode
				theme = DARK_MODE;
				//Theme has probably changed, so:

				//Redraw window
				InvalidateRect(hwnd, NULL, TRUE); 
				//Save theme to scrape.ini
				wss_buffer.str(L"");
				wss_buffer.clear();
				wss_buffer << theme;
				WritePrivateProfileString(L"General", L"theme", wss_buffer.str().c_str(), exeFullPath);
				break;
			default:
				return DefWindowProc(hwnd, message, wParam, lParam);
		}
		break;

	case WM_SIZE:
		//Get window dimensions and save to windowrect
		GetClientRect(hwnd, &windowrect);

		// Change position of the "Launch Squeak" button to account for the new window size
		LaunchSqueakButton = GetDlgItem(leftPanel, ID_BUTTON);
        SetWindowPos(LaunchSqueakButton, NULL, (int)windowrect.right * 0.65, windowrect.bottom * 0.8, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

		// Redraw window, window size has changed
        InvalidateRect(hwnd, NULL, TRUE); 
        break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	// Get the full path of the executable, this will be helpful for not writing scrape.ini to the wrong directory
	GetModuleFileName(NULL, exeFullPath, 256);

	// Find the last backslash in the path
	wchar_t* lastSlash = wcsrchr(exeFullPath, L'\\');
	if (lastSlash != NULL)
	{
		// Terminate the string after the last backslash, removing the executable's filename
		*(lastSlash + 1) = L'\0';
	}

	// Append scrape.ini to the path
	wcscat(exeFullPath, L"scrape.ini");


	   LPCWSTR MAINWC_NAME = L"Main Window Class";

	   WNDCLASS mainwc = { };

	   mainwc.lpfnWndProc = WndProc;
	   mainwc.hInstance = hInst;
	   mainwc.lpszClassName = MAINWC_NAME;

	   LPCWSTR LEFTWC_NAME = L"Left Window Class";

	   WNDCLASS leftwc = { };

	   leftwc.lpfnWndProc = LeftWndProc;
	   leftwc.hInstance = hInst;
	   leftwc.lpszClassName = LEFTWC_NAME;

	   LPCWSTR RIGHTWC_NAME = L"Right Window Class";

	   WNDCLASS rightwc = { };

	   rightwc.lpfnWndProc = RightWndProc;
	   rightwc.hInstance = hInst;
	   rightwc.lpszClassName = RIGHTWC_NAME;

	   if ((!RegisterClass(&mainwc) || !RegisterClass(&leftwc) || !RegisterClass(&rightwc)))
	   {
		   MessageBox(NULL, L"Window registration failed!\nError 0x0000000", L"Error!", MB_OK | MB_ICONERROR);
		   exit(EXIT_FAILURE);
	   }


	   //If scrape.ini doesn't exist, don't load it
	   if (FileExists(L".\\scrape.ini")) {
		   //Load stuff from INI file

		   ///Load theme from ini file
		   const int bufferSize = 384;
		   WCHAR lpwstr_buffer[bufferSize];
		   GetPrivateProfileString(L"General", L"theme", L"1", lpwstr_buffer, bufferSize, exeFullPath);
	
		   ///Copy to theme variable
		   wss_buffer.str(L"");
		   wss_buffer.clear();

		   wss_buffer << lpwstr_buffer;
		   wss_buffer >> theme;

		   GetPrivateProfileString(L"General", L"squeakpath", L"path_error", lpwstr_buffer, bufferSize - 2, exeFullPath);

		   if (!(wcscmp(lpwstr_buffer, L"path_error") == 0)) {
				// There's a path to a Squeak EXE in the INI file!

			    // We should probably copy it to the SqueakPath variable 
				wcscpy(SqueakPath, lpwstr_buffer);

			    // and tell the rest of the program that we don't need to declare a path anymore.
				SqueakPathDefined = true;

				//This fixes a bug for some reason
				WritePrivateProfileString(L"General", L"squeakpath", SqueakPath, exeFullPath);
		   }

		   ///Get the number of builds we need to load from the ini file
		   GetPrivateProfileString(L"General", L"numbuilds", L"65535", lpwstr_buffer, bufferSize, exeFullPath);
		   
		   ///Copy to a buffer
		   wss_buffer.str(L"");
		   wss_buffer.clear();

		   wss_buffer << lpwstr_buffer;
		   wss_buffer >> int_buffer;	
		   //If we can't find "numbuilds", just don't load the build list
		   if (int_buffer != 65535) {
			   //Load all of the build names and paths from the INI file
			   for (int i = 0; i < int_buffer; i++) {
				   //Clear data of temp
				   temp.name = L"";
				   temp.path = L"";

				   //Clear data of wss_buffer
				   wss_buffer.str(L"");
				   wss_buffer.clear();
				   

				   wss_buffer << "Build" << i;
				   wss_buffer >> wstring_buffer;
				   
				   //Load the "name" and "path" keys into the temp variable
				   GetPrivateProfileString(wstring_buffer.c_str(), L"name", L"Error! Report this to the devs.", lpwstr_buffer, bufferSize, exeFullPath);	
				   temp.name = lpwstr_buffer;
				   GetPrivateProfileString(wstring_buffer.c_str(), L"path", L"Error! Report this to the devs.", lpwstr_buffer, bufferSize, exeFullPath);
				   temp.path = lpwstr_buffer;
				   //Add temp to the builds vector
				   builds.push_back(temp);
			   }
		   }
	   }

	   //The path of the EXE is not defined, we should ask the user for it
	   if(!SqueakPathDefined) {
			MessageBox(NULL, L"Squeak executable not found! \n Please specify the path of a Squeak executable.", L"Info", MB_OK | MB_ICONINFORMATION);
			ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.lpstrFile = szFile;
            ofn.lpstrFile[0] = L'\0';
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = L"Executable\0*.EXE\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if(GetOpenFileName(&ofn) == TRUE)
            {
				wcscpy(SqueakPath, ofn.lpstrFile);
				SqueakPathDefined = true;
				//Write Squeak EXE path to the INI file
				
			}
			WritePrivateProfileString(L"General", L"squeakpath", SqueakPath, exeFullPath);
	   }
	   hMenuBar = LoadMenu(hInst, MAKEINTRESOURCE(IDC_SCRATCHETUS_CPP));
	   HWND hwnd = CreateWindow(MAINWC_NAME, L"Scrape Launcher", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, hMenuBar, hInst, NULL);

	   if (hwnd == NULL)
	   {
		   MessageBox(NULL, L"Window creation failed!\nError 0x0000001", L"Error!", MB_OK | MB_ICONERROR);
		   exit(EXIT_FAILURE);
	   }


	   int scrollbarWidth = GetSystemMetrics(SM_CXVSCROLL);
	   int titleBarHeight = GetSystemMetrics(SM_CYCAPTION);
	   int menuHeight = GetSystemMetrics(SM_CYMENU);
	   leftPanel = CreateWindowEx(WS_EX_CLIENTEDGE, LEFTWC_NAME, NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL, 0, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT - titleBarHeight - menuHeight, hwnd, NULL, hInstance, NULL);
	   rightPanel = CreateWindowEx(WS_EX_CLIENTEDGE, RIGHTWC_NAME, NULL, WS_CHILD | WS_VISIBLE, (WINDOW_WIDTH / 2), 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT - titleBarHeight - menuHeight, hwnd, NULL, hInstance, NULL);
	   ShowWindow(hwnd, nCmdShow);
	   UpdateWindow(hwnd);


	   MSG msg = { };
	   while (GetMessage(&msg, NULL, 0, 0))
	   {
		   TranslateMessage(&msg);
		   DispatchMessage(&msg);
	   }

	   return 0;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hwnd;

   hInst = hInstance; // Store instance handle in our global variable

   hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      0, 0, 640, 480, NULL, NULL, hInstance, NULL);

   if (!hwnd)
   {
      return FALSE;
   }

   ShowWindow(hwnd, nCmdShow);
   UpdateWindow(hwnd);

   return TRUE;
}
