// TODO: Get into alpha stage, then URGENT CLEANUP

#include "stdafx.h"
#include "Scratchetus_CPP.h"

#include <shellapi.h>
#include <string>
#include <commdlg.h>
#include <vector>
#include <windowsx.h>

#define MAX_LOADSTRING 100
#define LEFTSIDE_BRIGHTNESS 245
#define ID_BUTTON 1001

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
HWND				LaunchSqueakButton;

//declaring stuff used in the function
HBRUSH				hBrush;
HBRUSH				hBrush2;
RECT                windowrect;
RECT				leftsiderect;
RECT				buildrect;

// "Add Build" dialog stuff
wchar_t* filename;

wchar_t szFile[260];
OPENFILENAME ofn;


// struct to hold build name and path
struct BuildInfo
{
    std::wstring name;
    std::wstring path;
};

BuildInfo temp;

std::vector<BuildInfo> builds;

POINT point;
LPCWSTR lpcwstr_buffer;
short int selected_build = NULL;

bool FileExists(LPCWSTR fileName)
{
    DWORD fileAttr;

    fileAttr = GetFileAttributes(fileName);
    if (0xFFFFFFFF == fileAttr && GetLastError() == ERROR_FILE_NOT_FOUND)
    {
        return false;
    }
    return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
				lpcwstr_buffer = builds[selected_build].name.c_str();
				ShellExecute(NULL, L"open", L".\\res\\squeak.exe", lpcwstr_buffer, NULL, SW_SHOWDEFAULT);
				break;
			case IDM_ADDBUILD:
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hWnd;
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

				InvalidateRect(hWnd, NULL, TRUE);

				break;
			case IDM_ABOUT:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				break;
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_SIZE:
		GetClientRect(hWnd, &windowrect);
		LaunchSqueakButton = GetDlgItem(hWnd, ID_BUTTON);
        // Set the new position of the button
        SetWindowPos(LaunchSqueakButton, NULL, (int)windowrect.right * 0.65, windowrect.bottom * 0.8, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        InvalidateRect(hWnd, NULL, TRUE); // redraw window, window size has changed
        break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// create a grey brush
        hBrush = CreateSolidBrush(RGB(LEFTSIDE_BRIGHTNESS, LEFTSIDE_BRIGHTNESS, LEFTSIDE_BRIGHTNESS));
		hBrush2 = CreateSolidBrush(RGB(LEFTSIDE_BRIGHTNESS - 32, LEFTSIDE_BRIGHTNESS - 32, LEFTSIDE_BRIGHTNESS - 32));
		
		leftsiderect = windowrect;
		leftsiderect.right >>= 1;

        // fill the rectangle with the grey brush
        FillRect(hdc, &leftsiderect, hBrush);

		for(size_t i = 0; i < builds.size(); i++) {
			buildrect.left = 0;
			buildrect.top = i * 64;
			buildrect.right = leftsiderect.right; // Half the window's width
			buildrect.bottom = 64 + (i * 64); // Fixed height

			// Fill the rectangle with the grey brush
			FillRect(hdc, &buildrect, hBrush2);
			DrawTextW(hdc, (builds[i].name).c_str(), -1 /*null terminated*/, &buildrect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
		EndPaint(hWnd, &ps);
		break;
    case WM_LBUTTONDOWN:
		point.x = GET_X_LPARAM(lParam);
		point.y = GET_Y_LPARAM(lParam);

		for(size_t i = 0; i < builds.size(); ++i) 
		{
			RECT buildrect;
			buildrect.left = 0;
			buildrect.top = i * 64;
			buildrect.right = leftsiderect.right; // Half the window's width
			buildrect.bottom = 64 + (i * 64); // Fixed height

			if(PtInRect(&buildrect, point))
			{
				selected_build = i;
				lpcwstr_buffer = builds[i].name.c_str();
				break;
			}
		}
		break;
	case WM_CREATE:
	{
		// Create a button on the right side of the window
		CreateWindow(L"BUTTON", L"Launch Squeak", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
		windowrect.right << 2, 250, 125, 30, hWnd, (HMENU)ID_BUTTON, NULL, NULL);
	}
	break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
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

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SCRATCHETUS_CPP, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SCRATCHETUS_CPP));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCRATCHETUS_CPP));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_SCRATCHETUS_CPP);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      0, 0, 640, 480, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}