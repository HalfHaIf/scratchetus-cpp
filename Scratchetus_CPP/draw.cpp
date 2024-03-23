#include "draw.h"
#include "stdafx.h"
#include "font.h"
#include "Scratchetus_CPP.h"

#include <vector>


#define LIGHT_MODE 1
#define DARK_MODE 2

#define NUM_THEMES 2

/// struct to hold build name and path
struct BuildInfo
{
    std::wstring name;
    std::wstring path;
};

extern std::vector<BuildInfo> builds;

//declaring stuff used in the draw function
HBRUSH				LeftSideBrush;
HBRUSH				BuildBrush;
HBRUSH				BuildBrush2;
HBRUSH				SelectedBuildBrush;

extern RECT windowrect;

RECT				leftsiderect;
RECT				buildrect;

extern short int selected_build;

extern short int theme;

// Create a font
HFONT hFont = CreateFont(
    20,                        // Height of font
    0,                         // Average character width
    0,                         // Angle of escapement
    0,                         // Base-line orientation angle
    FW_DONTCARE,               // Font weight
    FALSE,                     // Italic attribute option
    FALSE,                     // Underline attribute option
    FALSE,                     // Strikeout attribute option
    DEFAULT_CHARSET,           // Character set identifier
    OUT_DEFAULT_PRECIS,        // Output precision
    CLIP_DEFAULT_PRECIS,       // Clipping precision
    DEFAULT_QUALITY,           // Output quality
    DEFAULT_PITCH | FF_DONTCARE, // Pitch and family
    L"Trebuchet MS");          // Font name


void draw(HDC hdc, HWND hwnd, PAINTSTRUCT ps)
{
	// Add the Trebuchet MS font
	AddTTFFile(IDR_BINARY1);

	hdc = BeginPaint(hwnd, &ps);

	SelectObject(hdc, hFont);

	if (theme == DARK_MODE) {
		//dark mode
		LeftSideBrush = CreateSolidBrush(RGB(44, 44, 58));
		BuildBrush = CreateSolidBrush(RGB(49, 49, 65));
		BuildBrush2 = CreateSolidBrush(RGB(47, 47, 63));
		SelectedBuildBrush = CreateSolidBrush(RGB(35, 35, 46));
		//set text to color to white in dark mode
		SetTextColor(hdc, RGB(255, 255, 255));
	}
	else
	{
		//light mode
		LeftSideBrush = CreateSolidBrush(RGB(LEFTSIDE_BRIGHTNESS, LEFTSIDE_BRIGHTNESS, LEFTSIDE_BRIGHTNESS));
		BuildBrush = CreateSolidBrush(RGB(LEFTSIDE_BRIGHTNESS - 32, LEFTSIDE_BRIGHTNESS - 32, LEFTSIDE_BRIGHTNESS - 32));
		BuildBrush2 = CreateSolidBrush(RGB(LEFTSIDE_BRIGHTNESS - 38, LEFTSIDE_BRIGHTNESS - 38, LEFTSIDE_BRIGHTNESS - 38));
		SelectedBuildBrush = CreateSolidBrush(RGB(LEFTSIDE_BRIGHTNESS - 48, LEFTSIDE_BRIGHTNESS - 48, LEFTSIDE_BRIGHTNESS - 48));
	}

		GetClientRect(hwnd, &leftsiderect);
        //Fill the rectangle with the grey brush
        FillRect(hdc, &leftsiderect, LeftSideBrush);
		
		int totalHeight = builds.size() * 32;

		// Set the vertical scroll bar range and page size
		SCROLLINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(si);
		si.fMask  = SIF_RANGE | SIF_PAGE;
		si.nMin   = 0;
		si.nMax   = totalHeight + 64;
		si.nPage  = 480; // Set this to the height of your client area
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

		// Get the vertical scroll position		
		int scrollPos = GetScrollPos(hwnd, SB_VERT); 


		if (builds.size() == 0) {
			SetBkMode(hdc, TRANSPARENT);
			DrawTextW(hdc, L"There are no builds at the moment. \n Why not add some?", -1 /*null terminated*/, &leftsiderect, DT_CENTER | DT_VCENTER);
			SetBkMode(hdc, OPAQUE);
		}
		else
		{
			//Iterate through all of the builds in the builds vector and draw a square containing the build name of each one
			for(size_t i = 0; i < builds.size(); i++) {
				buildrect.left = 0;
				buildrect.top = i * 32 - scrollPos;
				buildrect.right = leftsiderect.right; // Half the window's width
				buildrect.bottom = (32 + (i * 32)) - scrollPos; // Fixed height

				if (i != selected_build) {
					// Every second build, slightly darken the brush
					if (!(i & 0x01)) {
						FillRect(hdc, &buildrect, BuildBrush);
					}
					else
					{
						FillRect(hdc, &buildrect, BuildBrush2);
					}
				}
				else
				{
					FillRect(hdc, &buildrect, SelectedBuildBrush);
				}
				SetBkMode(hdc, TRANSPARENT);
				DrawTextW(hdc, (builds[i].name).c_str(), -1 /*null terminated*/, &buildrect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
				SetBkMode(hdc, OPAQUE);
			}
		 }
		EndPaint(hwnd, &ps);

		return;
}