#include "draw.h"
#include "stdafx.h"
#include "Scratchetus_CPP.h"

#include <vector>

/// struct to hold build name and path
struct BuildInfo
{
    std::wstring name;
    std::wstring path;
};

extern std::vector<BuildInfo> builds;

//declaring stuff used in the draw function
HBRUSH				hBrush;
HBRUSH				hBrush2;
extern RECT windowrect;
RECT				leftsiderect;
RECT				buildrect;

void draw(HDC hdc, HWND hwnd, PAINTSTRUCT ps)
{
	hdc = BeginPaint(hwnd, &ps);
	// create a grey brush
    hBrush = CreateSolidBrush(RGB(LEFTSIDE_BRIGHTNESS, LEFTSIDE_BRIGHTNESS, LEFTSIDE_BRIGHTNESS));
	hBrush2 = CreateSolidBrush(RGB(LEFTSIDE_BRIGHTNESS - 32, LEFTSIDE_BRIGHTNESS - 32, LEFTSIDE_BRIGHTNESS - 32));
		
	leftsiderect = windowrect;
	leftsiderect.right >>= 1;

        //Fill the rectangle with the grey brush
        FillRect(hdc, &leftsiderect, hBrush);
		
		//Iterate through all of the builds in the builds vector and draw a square containing the build name of each one
		for(size_t i = 0; i < builds.size(); i++) {
			buildrect.left = 0;
			buildrect.top = i * 64;
			buildrect.right = leftsiderect.right; // Half the window's width
			buildrect.bottom = 64 + (i * 64); // Fixed height

			// Fill the rectangle with the grey brush
			FillRect(hdc, &buildrect, hBrush2);
			DrawTextW(hdc, (builds[i].name).c_str(), -1 /*null terminated*/, &buildrect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
		EndPaint(hwnd, &ps);
		return;
}