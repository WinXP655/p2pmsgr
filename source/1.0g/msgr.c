// Experimental version - adding GUI to 1.0

#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <process.h>
#include <stdbool.h>

#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "user32.lib")

#define PORT 8888
#define BUFFER_SIZE 1024
#define ID_EDIT 101
#define ID_BUTTON 102
#define ID_LIST 103
#define IDC_IP 1001

SOCKET client_socket = INVALID_SOCKET;
HWND hEdit, hButton, hList;
int is_running = 1;

// Функция для приема сообщений
void receive_messages(void* arg) {
    char buffer[BUFFER_SIZE];
    while (is_running) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes <= 0) {
            MessageBox(NULL, "Connection lost", "Error", MB_OK);
            is_running = 0;
            break;
        }
        
        // Добавляем сообщение в список
        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)buffer);
    }
    _endthread();
}

BOOL CALLBACK IPDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static char* ip;
    
    switch (msg) {
        case WM_INITDIALOG:
            ip = (char*)lParam;
            return TRUE;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                GetDlgItemText(hwnd, IDC_IP, ip, 16);
                EndDialog(hwnd, IDOK);
            } else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwnd, IDCANCEL);
            }
            return TRUE;
    }
    return FALSE;
}

// Оконная процедура
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            // Создаем элементы интерфейса
            hList = CreateWindow("LISTBOX", "", 
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
                10, 10, 360, 200, hwnd, (HMENU)ID_LIST, NULL, NULL);
            
            hEdit = CreateWindow("EDIT", "", 
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                10, 220, 280, 25, hwnd, (HMENU)ID_EDIT, NULL, NULL);
            
            hButton = CreateWindow("BUTTON", "Send", 
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                300, 220, 70, 25, hwnd, (HMENU)ID_BUTTON, NULL, NULL);
            
            return 0;
        }
        
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BUTTON) {
                char buffer[BUFFER_SIZE];
                GetWindowText(hEdit, buffer, BUFFER_SIZE);
                
                if (strlen(buffer) > 0) {
                    send(client_socket, buffer, strlen(buffer), 0);

                    // Формируем строку "You: ..."
                    char msgbuf[BUFFER_SIZE + 10];
                    sprintf(msgbuf, "You: %s", buffer);

                    SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)msgbuf);

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
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Функция инициализации сети
int init_network(bool is_server) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(1,1), &wsa) != 0) {
        MessageBox(NULL, "WSAStartup failed", "Error", MB_OK);
        return 1;
    }

    if (is_server) {
        SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);

        bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
        listen(server_fd, 1);

        MessageBox(NULL, "Waiting for connection...", "Server", MB_OK);
        client_socket = accept(server_fd, NULL, NULL);
        closesocket(server_fd);
    } else {
        char server_ip[16] = "127.0.0.1"; // По умолчанию localhost
        
        // Диалог ввода IP
        if (DialogBoxParam(GetModuleHandle(NULL), 
            MAKEINTRESOURCE(1), NULL, (DLGPROC)IPDialogProc, (LPARAM)server_ip) != IDOK) {
            return 1;
        }

        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = inet_addr(server_ip);

        if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
            MessageBox(NULL, "Connection failed", "Error", MB_OK);
            return 1;
        }
    }
    
    _beginthread(receive_messages, 0, NULL);
    return 0;
}

// Точка входа
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Диалог выбора режима
    int mode = MessageBox(NULL, "Run as server?", "Chat", MB_YESNOCANCEL);
    if (mode == IDCANCEL) return 0;
    
    if (init_network(mode == IDYES)) return 1;

    // Создание окна
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "ChatWindowClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindow("ChatWindowClass", "Chat Application",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Цикл сообщений
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}