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

// Forward declarations of functions included in this code module
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

//HWND that stores the button to launch Squeak
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

LPWSTR lpwstr_buffer;

//Stores which build we've selected
short int selected_build = 65535;

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
			case ID_BUTTON:
				//Run squeak and pass the name of the currently selected build to it
				lpcwstr_buffer = builds[selected_build].name.c_str();
				if (selected_build != 65535) {
					//We have selected a build, let's open it!
					ShellExecute(NULL, L"open", L".\\squeak.exe", lpcwstr_buffer, NULL, SW_SHOWDEFAULT);
				}
				else
				{
					//No build selected, which one are we supposed to open?
					MessageBox(NULL, L"No build selected! Select a build first.", L"Info", MB_OK | MB_ICONINFORMATION);
				}
				break;
			case IDM_ADDBUILD:
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
					
					filename = wcsrchr(ofn.lpstrFile, L'\\');
					if(filename != NULL) {
						temp.name = filename + 1;
					}
					
					builds.push_back(temp);
                }
				
				WritePrivateProfileString(L"General", L"numbuilds", builds.size(), L".\\scrape.ini");

				for (int i = 0; i < builds.size(); i++)
				{
					std::wstringstream wss;
					wss << L"Build" << i;
					std::wstring section = wss.str();
					WritePrivateProfileString(section.c_str(), L"name", builds[i].name.c_str(), L".\\scrape.ini");
					WritePrivateProfileString(section.c_str(), L"path", builds[i].path.c_str(), L".\\scrape.ini");
				}

				InvalidateRect(hwnd, NULL, TRUE);

				break;
			case IDM_ABOUT:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
				break;
			case IDM_EXIT:
				DestroyWindow(hwnd);
				break;
			default:
				return DefWindowProc(hwnd, message, wParam, lParam);
		}
		break;

	case WM_SIZE:
		//Get window dimensions and save to windowrect
		GetClientRect(hwnd, &windowrect);

		// Change position of the "Launch Squeak" button to account for the new window size
		LaunchSqueakButton = GetDlgItem(hwnd, ID_BUTTON);
        SetWindowPos(LaunchSqueakButton, NULL, (int)windowrect.right * 0.65, windowrect.bottom * 0.8, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

		// Redraw window, window size has changed
        InvalidateRect(hwnd, NULL, TRUE); 
        break;
	case WM_PAINT:
		draw(hdc, hwnd, ps);
		break;
    case WM_LBUTTONDOWN:
		mousecoords.x = GET_X_LPARAM(lParam);
		mousecoords.y = GET_Y_LPARAM(lParam);

		for(size_t i = 0; i < builds.size(); ++i) 
		{
			RECT buildrect;
			buildrect.left = 0;
			buildrect.top = i * 64;
			buildrect.right = windowrect.right << 1; // Half the window's width
			buildrect.bottom = 64 + (i * 64); // Fixed height

			if(PtInRect(&buildrect, mousecoords))
			{
				selected_build = i;
				lpcwstr_buffer = builds[i].name.c_str();
				break;
			}
		}
		InvalidateRect(hwnd, NULL, TRUE); // need to redraw the window, selected build has probably changed
		break;
	case WM_CREATE:
	{
		// Create a button on the right side of the window
		CreateWindow(L"BUTTON", L"Launch Squeak", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
		windowrect.right << 2, 250, 125, 30, hwnd, (HMENU)ID_BUTTON, NULL, NULL);
	}
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
   LPCWSTR CLASS_NAME = L"Main Window Class";

   WNDCLASS wc = { };

   wc.lpfnWndProc = WndProc;
   wc.hInstance = hInst;
   wc.lpszClassName = CLASS_NAME;

   if (!RegisterClass(&wc))
   {
       MessageBox(NULL, L"Window registration failed!\nError 0x0000000", L"Error!", MB_OK | MB_ICONERROR);
       exit(EXIT_FAILURE);
   }

   HMENU hMenuBar = LoadMenu(hInst, MAKEINTRESOURCE(IDC_SCRATCHETUS_CPP));
   HWND hwnd = CreateWindow(CLASS_NAME, L"Scrape Launcher", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 600, 400, NULL, hMenuBar, hInst, NULL);

   if (hwnd == NULL)
   {
       MessageBox(NULL, L"Window creation failed!\nError 0x0000001", L"Error!", MB_OK | MB_ICONERROR);
       exit(EXIT_FAILURE);
   }
   if (!FileExists(L"squeak.exe"))
   {
       MessageBox(NULL, L"'squeak.exe' not found!\nError 0x0000002", L"Error!", MB_OK | MB_ICONERROR);
       exit(EXIT_FAILURE);
   }

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
