#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

#pragma comment(lib, "user32.lib")

void PlaySoundStopped() {
    // Windows API to play sound
    PlaySound(TEXT("srvstop.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void PlaySoundStopError() {
    // Windows API to play sound
    PlaySound(TEXT("srvstoperr.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

BOOL StopServerProcess() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    BOOL bFound = FALSE;
    if (Process32First(hSnapshot, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, "p2pmsgr.exe") == 0) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProcess != NULL) {
                    // Try to close gracefully first
                    HWND hWnd = FindWindow(NULL, "P2P Messenger Server");
                    if (hWnd) {
                        PostMessage(hWnd, WM_CLOSE, 0, 0);
                        
                        // Wait for graceful exit
                        if (WaitForSingleObject(hProcess, 5000) == WAIT_OBJECT_0) {
                            CloseHandle(hProcess);
                            CloseHandle(hSnapshot);
                            return TRUE;
                        }
                    }
                    
                    // Force termination if graceful close failed
                    if (TerminateProcess(hProcess, 0)) {
                        CloseHandle(hProcess);
                        CloseHandle(hSnapshot);
                        return TRUE;
                    }
                    CloseHandle(hProcess);
                }
                bFound = TRUE;
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return !bFound; // Return TRUE if process not found (technically success)
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    if (StopServerProcess()) {
        MessageBox(NULL, 
            "P2P Messenger Server stopped successfully", 
            "P2P Messenger Server Stop", 
            MB_OK | MB_ICONINFORMATION);
    } else {
        DWORD err = GetLastError();
        char msg[256];
        sprintf(msg, "Failed to stop P2P Messenger Server.\nError: %d", err);
        
        MessageBox(NULL, 
            msg, 
            "P2P Messenger Server Stop", 
            MB_OK | MB_ICONERROR);
        return 1;
    }
    return 0;
}