#include <windows.h>
#include <cstdlib>
#include "stdafx.h"
#include "font.h" 

void AddTTFFile(int resourceid)
{

    HINSTANCE hInstance = ::GetModuleHandle(NULL); // get the handle to the current module
    HRSRC hFontResource = FindResource(hInstance, MAKEINTRESOURCE(resourceid), L"BINARY"); // find the resource


    if (hFontResource == NULL)
    {
        MessageBox(NULL, L"Failed to find the font resource. \nError 0x0000002", L"Error", MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }

    HGLOBAL hLoadedResource = LoadResource(hInstance, hFontResource); // load the resource

    if (hLoadedResource == NULL)
    {
        MessageBox(NULL, L"Failed to load the font resource. \nError 0x0000003", L"Error", MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }

    void* FntData = LockResource(hLoadedResource); // lock the resource

    if (!FntData)
    {
        MessageBox(NULL, L"Failed to lock the font resource. \nError 0x0000004", L"Error", MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }

    DWORD len = SizeofResource(hInstance, hFontResource); // get the size of the resource
    DWORD nFonts = 0;

    HANDLE testHandle = AddFontMemResourceEx(FntData, len, NULL, &nFonts); // add the font

    if (!testHandle)
    {
        MessageBox(NULL, L"Failed to add the font. \nError 0x0000005", L"Error", MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }

    return;
}