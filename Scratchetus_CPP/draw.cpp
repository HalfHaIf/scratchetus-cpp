#include "draw.h"
#include "stdafx.h"
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
HBRUSH				RightSideBrush;
HBRUSH				LeftSideBrush;
HBRUSH				BuildBrush;
HBRUSH				SelectedBuildBrush;

extern RECT windowrect;

RECT				leftsiderect;
RECT				buildrect;

extern short int selected_build;

extern short int theme;

void draw(HDC hdc, HWND hwnd, PAINTSTRUCT ps)
{
	hdc = BeginPaint(hwnd, &ps);
	if (theme == DARK_MODE) {
		//dark mode
		RightSideBrush = CreateSolidBrush(RGB(51, 51, 67));
		LeftSideBrush = CreateSolidBrush(RGB(44, 44, 58));
		BuildBrush = CreateSolidBrush(RGB(47, 47, 63));
		SelectedBuildBrush = CreateSolidBrush(RGB(35, 35, 46));	
	}
	if (theme == LIGHT_MODE) {
		//light mode
		RightSideBrush = CreateSolidBrush(RGB(255, 255, 255));
		LeftSideBrush = CreateSolidBrush(RGB(LEFTSIDE_BRIGHTNESS, LEFTSIDE_BRIGHTNESS, LEFTSIDE_BRIGHTNESS));
		BuildBrush = CreateSolidBrush(RGB(LEFTSIDE_BRIGHTNESS - 32, LEFTSIDE_BRIGHTNESS - 32, LEFTSIDE_BRIGHTNESS - 32));
		SelectedBuildBrush = CreateSolidBrush(RGB(LEFTSIDE_BRIGHTNESS - 48, LEFTSIDE_BRIGHTNESS - 48, LEFTSIDE_BRIGHTNESS - 48));
	}

	leftsiderect = windowrect;
	leftsiderect.right >>= 1;

		FillRect(hdc, &windowrect, RightSideBrush);
        //Fill the rectangle with the grey brush
        FillRect(hdc, &leftsiderect, LeftSideBrush);
		
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
				buildrect.top = i * 64;
				buildrect.right = leftsiderect.right; // Half the window's width
				buildrect.bottom = 64 + (i * 64); // Fixed height

				if (i != selected_build) {
					FillRect(hdc, &buildrect, BuildBrush);
				}
				else
				{
					FillRect(hdc, &buildrect, SelectedBuildBrush);
				}
				SetBkMode(hdc, TRANSPARENT);
				DrawTextW(hdc, (builds[i].name).c_str(), -1 /*null terminated*/, &buildrect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				SetBkMode(hdc, OPAQUE);
			}
		 }
		EndPaint(hwnd, &ps);
		return;
}