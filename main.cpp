#include <iostream>
#include <iomanip>
#include <string>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

void report_error(const TCHAR * function, DWORD error_code = GetLastError())
{
    std::cout << function << ": error " << std::hex << error_code << " happened";
    TCHAR * message = reinterpret_cast<TCHAR *>(&message);
    DWORD result = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, error_code, 0, message, 128, NULL);
    if( result ) {
        std::cout << ": " << message << std::endl;
        if(LocalFree(message)!=NULL){
            std::cout << "LocalFree failed, PANIC!!!!1111";
            exit(1);
        }
    }
    else {
        std::cout << " and FormatMessage failed" << std::endl;
    }
}
#if (defined __FILE__) and (defined __LINE__)
#define OUTPUT_FILE_LINE {std::cout<<__FILE__ <<":"<<__LINE__<<" ";}
#else
#define OUTPUT_FILE_LINE
#endif

#define REPORT_ERROR(function) { \
    OUTPUT_FILE_LINE             \
    report_error(function);      \
}

#define REPORT_ERROR_CODE(function, code) { \
    OUTPUT_FILE_LINE                        \
    report_error(function, code);           \
}

int main()
{
    HKEY hKey;
    DWORD result = RegOpenKeyEx(HKEY_CURRENT_USER,
                                "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                                0,
                                KEY_SET_VALUE,
                                &hKey);
    if( (result != ERROR_SUCCESS) || (hKey == NULL) ) {
        REPORT_ERROR_CODE("RegOpenKeyEx", result);
    } else {
        TCHAR exectutable_path[MAX_PATH];
        DWORD path_length = GetModuleFileName( NULL, exectutable_path, MAX_PATH );
        if( path_length == 0 ||  MAX_PATH <= path_length ) {
            REPORT_ERROR("GetModuleFileName");
        } else {
            LONG result = RegSetValueEx(hKey, "My program", 0, REG_SZ, (LPBYTE)exectutable_path, path_length);
            if( result != ERROR_SUCCESS ){
                REPORT_ERROR("RegSetValueEx");
            }
        }
        LONG result = RegCloseKey(hKey);
        if( result != ERROR_SUCCESS ){
            REPORT_ERROR("RegCloseKey");
        }
    }
    TCHAR path[MAX_PATH];
    DWORD path_length = GetCurrentDirectory(MAX_PATH, path);
    if( path_length == 0 || MAX_PATH <= path_length ) {
        REPORT_ERROR("GetCurrentDirectory");
    } else {
        std::cout << "Current path: '" << path << "'" << std::endl;
    }
    MessageBox(NULL,"My program", "Done", MB_OK);
    return 0;
}
