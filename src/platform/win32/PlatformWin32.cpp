
#include "platform/Platform.h"
#include <filesystem>
#include <string>

#define WIN32_LEAN_AND_MEAN

#include "common/Filesystem.h"

#include <windows.h>
#include <commdlg.h>

namespace engine
{
    std::string Platform::FilePicker(const char* startIn = nullptr)
    {
        fs::Path pwd = std::filesystem::current_path();

        OPENFILENAME ofn;
        char filename[1024];

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;

        ofn.lpstrFile = filename;
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(filename);
        
        //open.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
        //open.nFilterIndex = 1;

        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;

        // Windows tends to override this for convenience anyway
        ofn.lpstrInitialDir = startIn;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        BOOL result = GetOpenFileName(&ofn);

        // Prevent the pwd from being changed
        std::filesystem::current_path(pwd);

        if (result) {
            return std::string(ofn.lpstrFile);
        } else {
            return std::string();
        }
    }
}