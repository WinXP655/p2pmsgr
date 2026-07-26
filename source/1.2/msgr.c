#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <process.h>
#include <stdbool.h>

#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")

#define PORT 8888
#define BUFFER_SIZE 1024
#define ID_EDIT 101
#define ID_SEND 102
#define ID_LIST 103
#define ID_USERS 104
#define IDC_IP 1001
#define ID_MSG_DISPLAY 105

SOCKET client_socket = INVALID_SOCKET;
HWND hEdit, hSendBtn, hMsgList, hUsersList, hMsgDisplay;
int is_running = 1;
char server_ip[16] = "127.0.0.1";
char computer_name[MAX_COMPUTERNAME_LENGTH + 1];
char peer_name[MAX_COMPUTERNAME_LENGTH + 1];
char peer_ip[16] = "";

// Function prototypes
void ReceiveMessages(void* arg);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ConnectDialogProc(HWND, UINT, WPARAM, LPARAM);
void InitializeNetwork(bool is_server);
void AddMessage(const char* msg);
void ShowMainWindow(HINSTANCE hInstance, int nCmdShow);
void GetLocalComputerName();  // Renamed to avoid conflict

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Get computer name once at startup
    GetLocalComputerName();

    int mode = MessageBox(NULL, "Run as server?", "Messenger", MB_YESNOCANCEL);
    if (mode == IDCANCEL) return 0;
    
    if (mode == IDYES) {
        // Server mode - initialize network first
        InitializeNetwork(true);
    } else {
        // Client mode - show connection dialog
        if (DialogBoxParam(hInstance, MAKEINTRESOURCE(1), NULL, ConnectDialogProc, 0) != IDOK) {
            return 0;
        }
        InitializeNetwork(false);
    }

    // Show GUI only after connection is established
    ShowMainWindow(hInstance, nCmdShow);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

// Get computer name (renamed to avoid conflict)
void GetLocalComputerName() {
    DWORD size = sizeof(computer_name);
    GetComputerNameA(computer_name, &size);
}

// Show main chat window
void ShowMainWindow(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MessengerClass";
    RegisterClass(&wc);

    char title[128];
    sprintf(title, "Messenger - %s", computer_name);
    
    HWND hWnd = CreateWindow("MessengerClass", title,
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
}

// Client connection dialog
INT_PTR CALLBACK ConnectDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_INITDIALOG:
            SetDlgItemText(hwnd, IDC_IP, "127.0.0.1");
            return TRUE;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                GetDlgItemText(hwnd, IDC_IP, server_ip, sizeof(server_ip));
                EndDialog(hwnd, IDOK);
            } else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwnd, IDCANCEL);
            }
            return TRUE;
    }
    return FALSE;
}

// Main window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // Create UI elements
            hUsersList = CreateWindow("LISTBOX", "", 
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | LBS_NOTIFY,
                10, 10, 150, 300, hWnd, (HMENU)ID_USERS, NULL, NULL);
            
            // Replace ListBox with read-only Edit control for messages
			hMsgDisplay = CreateWindow("EDIT", "", 
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | 
				ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
				170, 10, 400, 300, hWnd, (HMENU)ID_MSG_DISPLAY, NULL, NULL);
            
            hEdit = CreateWindow("EDIT", "", 
                WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_AUTOHSCROLL | ES_MULTILINE,
                170, 320, 350, 30, hWnd, (HMENU)ID_EDIT, NULL, NULL);
            
            hSendBtn = CreateWindow("BUTTON", "Send", 
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                530, 320, 50, 30, hWnd, (HMENU)ID_SEND, NULL, NULL);
            
            // Add users to list
            char user_info[128];
            sprintf(user_info, "%s (You)", computer_name);
            SendMessage(hUsersList, LB_ADDSTRING, 0, (LPARAM)user_info);
            
            if (peer_name[0] != '\0') {
                sprintf(user_info, "%s (%s)", peer_name, peer_ip);
                SendMessage(hUsersList, LB_ADDSTRING, 0, (LPARAM)user_info);
            }
            return 0;
        }
        
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_SEND) {
                char buffer[BUFFER_SIZE];
                GetWindowText(hEdit, buffer, BUFFER_SIZE);
                
                if (strlen(buffer) > 0) {
                    char full_msg[BUFFER_SIZE + 128];
                    sprintf(full_msg, "[%s]: %s", computer_name, buffer);
                    
                    send(client_socket, full_msg, strlen(full_msg), 0);
                    AddMessage(full_msg);
                    SetWindowText(hEdit, "");
                }
            }
            return 0;
        }
        
        case WM_DESTROY: {
            is_running = 0;
            closesocket(client_socket);
            WSACleanup();
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// Network initialization
void InitializeNetwork(bool is_server) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(1,1), &wsa);

    if (is_server) {
        SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server_addr = {0};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);

        bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
        listen(server_fd, 1);
        
        // Get server IP
        char hostname[256];
        gethostname(hostname, sizeof(hostname));
        struct hostent* host = gethostbyname(hostname);
        strcpy(server_ip, inet_ntoa(*(struct in_addr*)host->h_addr_list[0]));
        
        // Wait for connection
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);
        client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        closesocket(server_fd);
        
        // Get client info
        strcpy(peer_ip, inet_ntoa(client_addr.sin_addr));
        recv(client_socket, peer_name, sizeof(peer_name), 0);
        
        // Send server computer name
        send(client_socket, computer_name, strlen(computer_name)+1, 0);
    } 
    else {
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server_addr = {0};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = inet_addr(server_ip);

        if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0) {
            // Get server IP
            struct sockaddr_in server_info;
            int len = sizeof(server_info);
            getsockname(client_socket, (struct sockaddr*)&server_info, &len);
            strcpy(peer_ip, inet_ntoa(server_info.sin_addr));
            
            // Exchange computer names
            send(client_socket, computer_name, strlen(computer_name)+1, 0);
            recv(client_socket, peer_name, sizeof(peer_name), 0);
        }
    }
    
    _beginthread(ReceiveMessages, 0, NULL);
}

// Message receiving thread
void ReceiveMessages(void* arg) {
    char buffer[BUFFER_SIZE];
    
    while (is_running) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes <= 0) {
            AddMessage("Connection lost");
            is_running = 0;
            break;
        }
        AddMessage(buffer);
    }
    _endthread();
}

void AddMessage(const char* msg) {
    // Get current text length
    int len = GetWindowTextLength(hMsgDisplay);
    
    // Append new message with newline
    SendMessage(hMsgDisplay, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessage(hMsgDisplay, EM_REPLACESEL, FALSE, (LPARAM)msg);
    SendMessage(hMsgDisplay, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
    
    // Auto-scroll to bottom
    SendMessage(hMsgDisplay, WM_VSCROLL, SB_BOTTOM, 0);
}