
#include "platform/Platform.h"
#include <filesystem>
#include <string>
#include <codecvt>

#define WIN32_LEAN_AND_MEAN

#include "common/Filesystem.h"
#include "common/String.h"

#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>

namespace chisel
{
    std::string Platform::FilePicker(bool open, std::span<const ExtensionName> extensions, const char* startIn)
    {
        char filterString[512];

        fs::Path pwd = std::filesystem::current_path();

        OPENFILENAME ofn;
        char filename[1024];

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;

        ofn.lpstrFile = filename;
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(filename);

        if (extensions.size())
        {
            filterString[0] = '\0';

            for (const auto& extension : extensions)
            {
                auto ext = std::string(extension.ext);

                char localFilter[512];
                std::string lowerExt = ext;
                std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), [](unsigned char c) { return std::tolower(c); });
                auto lowerExtNames = str::split(lowerExt, ",");

                char formattedExts[512];
                formattedExts[0] = '\0';
                uint32_t i = 0;
                for (auto& ext : lowerExtNames)
                {
                    char formattedExt[512];
                    if (i == 0)
                        snprintf(formattedExt, sizeof(formattedExt), "%.*s", int(ext.length()), ext.data());
                    else
                        snprintf(formattedExt, sizeof(formattedExt), "} %.*s", int(ext.length()), ext.data());

                    strncat(formattedExts, formattedExt, sizeof(formattedExts));
                    i++;
                }

                snprintf(localFilter, sizeof(localFilter), "%.*s (%s);%s;", int(extension.prettyName.length()), extension.prettyName.data(), formattedExts, ext.c_str());
                strncat(filterString, localFilter, sizeof(localFilter));
            }

            // The API is so weird and wants null separated shit and then wants a double
            // null terminated thing. What a PITA.
            // Do some fixups to get what we want.
            for (size_t i = 0; i < sizeof(filterString); i++)
            {
                filterString[i] = filterString[i] == ';' ? '\0' : filterString[i];
                filterString[i] = filterString[i] == ',' ? ';' : filterString[i];
                filterString[i] = filterString[i] == '}' ? ',' : filterString[i];
            }   

            ofn.lpstrFilter = filterString;
            ofn.nFilterIndex = 1;
        }

        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;

        // Windows tends to override this for convenience anyway
        ofn.lpstrInitialDir = startIn;
        if (open)
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        BOOL result;
        if (open)
            result = GetOpenFileName(&ofn);
        else
            result = GetSaveFileName(&ofn);

        // Prevent the pwd from being changed
        std::filesystem::current_path(pwd);

        if (result) {
            auto path = std::string(ofn.lpstrFile);
            if (extensions.size())
            {
                if (!ofn.nFileExtension && ofn.nFilterIndex >= 1 && ofn.nFilterIndex <= extensions.size())
                {
                    path += ".";
                    std::string lowerExt = std::string(extensions[ofn.nFilterIndex - 1].ext);
                    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), [](unsigned char c) { return std::tolower(c); });
                    path += lowerExt;
                }
            }
            return path;
        } else {
            return std::string();
        }
    }

    std::string Platform::FolderPicker(const char* startIn)
    {
        IFileDialog* pfd;
        if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
        {
            if (startIn)
            {
                IShellItem* psi;
                std::wstring wstartIn = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(startIn);
                if (SUCCEEDED(SHCreateItemFromParsingName(wstartIn.c_str(), NULL, IID_PPV_ARGS(&psi))))
                {
                    pfd->SetFolder(psi);
                    psi->Release();
                }
            }

            DWORD dwOptions;
            if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
            {
                pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
            }

            if (SUCCEEDED(pfd->Show(NULL)))
            {
                IShellItem* psi;
                if (SUCCEEDED(pfd->GetResult(&psi)))
                {
                    PWSTR pszPath;
                    if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)))
                    {
                        std::wstring wstr(pszPath);
                        CoTaskMemFree(pszPath);
                        psi->Release();
                        pfd->Release();
                        return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wstr);
                    }
                    psi->Release();
                }
            }
            pfd->Release();
        }
        return std::string();
    }
}