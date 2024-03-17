#include "fileexists.h"
#include "stdafx.h"

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