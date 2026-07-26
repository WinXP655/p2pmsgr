#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <stdbool.h>
#include <time.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <unistd.h>
#include <mstcpip.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")

#define PORT 8888
#define BUFFER_SIZE 1024
#define ID_EDIT 101
#define ID_SEND 102
#define ID_LIST 103
#define ID_USERS 104
#define IDC_IP 1001
#define IDM_EXIT 2001
#define IDM_RESTART 2002
#define ID_MSG_DISPLAY 105
#define IDM_ABOUT 2003
#define IDM_CLEAR 2004
#define IDM_SAVE 2006

SOCKET client_socket = INVALID_SOCKET;
HWND hEdit, hSendBtn, hMsgList, hUsersList, hMsgDisplay;
int is_running = 1;
char server_ip[16] = "127.0.0.1";
char computer_name[MAX_COMPUTERNAME_LENGTH + 1];
char peer_name[MAX_COMPUTERNAME_LENGTH + 1];
char peer_ip[16] = "";
bool is_server = false;
FILE* chatlog = NULL;
WNDPROC g_oldEditProc = NULL;
HWND hwnd_global = NULL;

// Function prototypes
void ReceiveMessages(void* arg);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ConnectDialogProc(HWND, UINT, WPARAM, LPARAM);
void InitializeNetwork(bool server_mode);
void AddMessage(const char* msg);
void ShowMainWindow(HINSTANCE hInstance, int nCmdShow);
void GetLocalComputerName();
void CleanupAndExit(HWND hWnd, bool restart);
void ShowServerIPMessage();
void CreateMenuBar(HWND hWnd);
void LogMessage(const char* message);
void SendCurrentMessage(HWND hWnd);
LRESULT CALLBACK EditSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
void PlayNotificationSoundJoin();
void PlayNotificationSoundError();
int ShowConnectionDialog(HWND hwnd, BOOL isIncoming, const char* remoteName, const char* remoteIp);

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    GetLocalComputerName();

    int mode = MessageBox(NULL, "Run as server?\nYes - As Server\nNo - As Client (will be asked for server ip)\nCancel - Exit Messenger", "Messenger", MB_YESNOCANCEL | MB_ICONQUESTION);
    if (mode == IDCANCEL) return 0;
    
    is_server = (mode == IDYES);
    
    if (is_server) {
        // Open chat log file
        chatlog = fopen("chatlog.txt", "a");
        if (chatlog == NULL) {
			PlayNotificationSoundError();
            MessageBox(NULL, "Failed to open chat log file", "Error", MB_OK | MB_ICONERROR);
        } else {
            fprintf(chatlog, "\n=== New Session Started ===\n");
            fflush(chatlog);
        }
        InitializeNetwork(true);
    } else {
        if (DialogBoxParam(hInstance, MAKEINTRESOURCE(1), NULL, ConnectDialogProc, 0) != IDOK) {
            return 0;
        }
        InitializeNetwork(false);
    }

	PlayNotificationSoundJoin();
    ShowMainWindow(hInstance, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

void LogMessage(const char* message) {
    if (chatlog == NULL) return;
    
    time_t now;
    time(&now);
    struct tm* tm_info = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    fprintf(chatlog, "[%s] %s\n", timestamp, message);
    fflush(chatlog);
}

void GetLocalComputerName() {
    DWORD size = sizeof(computer_name);
    GetComputerNameA(computer_name, &size);
}

void ShowMainWindow(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MessengerClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    RegisterClass(&wc);

    char title[128];
    sprintf(title, "Messenger - %s", computer_name);
    
    HWND hWnd = CreateWindow("MessengerClass", title,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 435,
        NULL, NULL, hInstance, NULL);

	hwnd_global = hWnd;

    CreateMenuBar(hWnd);
    
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
}

void CreateMenuBar(HWND hWnd) {
    HMENU hMenu = CreateMenu();
    HMENU hSessionMenu = CreatePopupMenu();
	HMENU hHelpMenu = CreatePopupMenu();
    
    AppendMenu(hSessionMenu, MF_STRING, IDM_RESTART, "Restart Session");
    AppendMenu(hSessionMenu, MF_STRING, IDM_EXIT, "End Session");
	AppendMenu(hSessionMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hSessionMenu, MF_STRING, IDM_EXIT, "Exit");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSessionMenu, "Session");
	
	// Chat menu
    HMENU hChatMenu = CreatePopupMenu();
    AppendMenu(hChatMenu, MF_STRING, IDM_CLEAR, "Clear");
	AppendMenu(hChatMenu, MF_STRING, IDM_SAVE, "Save");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hChatMenu, "Chat");
    
	AppendMenu(hHelpMenu, MF_STRING, IDM_ABOUT, "About");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, "Help");
	
    SetMenu(hWnd, hMenu);
}

void FlashWindowOnMessage(HWND hWnd) {
    FLASHWINFO fi;
    fi.cbSize = sizeof(FLASHWINFO);
    fi.hwnd = hWnd;
    fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
    fi.uCount = 3;
    fi.dwTimeout = 0;
    FlashWindowEx(&fi);
}

void ShowServerIPMessage() {
    char message[256];
    sprintf(message, "Server IP: %s\nGive this to users to connect\nNote: To stop server, open Task Manager and exit p2pmsgr.exe", server_ip);
    MessageBox(NULL, message, "Messenger Server", MB_OK | MB_ICONINFORMATION);
}

void PlayNotificationSound() {
    // Windows API to play sound
    PlaySound(TEXT("newmsg.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void PlayNotificationSoundSystem() {
    // Windows API to play sound
    PlaySound(TEXT("system.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void PlayNotificationSoundLeft() {
    // Windows API to play sound
    PlaySound(TEXT("left.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void PlayNotificationSoundJoin() {
    // Windows API to play sound
    PlaySound(TEXT("join.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void PlayNotificationSoundError() {
    // Windows API to play sound
    PlaySound(TEXT("error.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

INT_PTR CALLBACK AboutDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MessageBox(NULL,
           "Simple P2P Messenger\n(c) WinXP655, 2025\nCreated with clean Windows API!",
           "About Messenger",
           MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK EditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_GETDLGCODE) {
        return DLGC_WANTALLKEYS | CallWindowProc(g_oldEditProc, hWnd, uMsg, wParam, lParam);
    }
    else if (uMsg == WM_KEYDOWN && wParam == VK_RETURN) {
        if (!(GetAsyncKeyState(VK_SHIFT) & 0x8000)) {
            // Post the message to avoid reentrancy issues
            PostMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(ID_SEND, 0), 0);
            return 0;  // Block the Enter key
        }
    }
    return CallWindowProc(g_oldEditProc, hWnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK ConnectDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_INITDIALOG:
            SetDlgItemText(hwnd, IDC_IP, "127.0.0.1");
            return TRUE;
            
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                char ip[16];
                GetDlgItemText(hwnd, IDC_IP, ip, sizeof(ip));
                
                // Trim whitespace
                char *p = ip;
                while (*p == ' ') p++;
                int len = strlen(p);
                while (len > 0 && p[len-1] == ' ') {
                    p[len-1] = '\0';
                    len--;
                }
                
                // Validate IP
                if (strlen(p) == 0) {
                    MessageBox(hwnd, "Server IP is required for connection", 
                             "Error", MB_OK | MB_ICONERROR);
                    SetFocus(GetDlgItem(hwnd, IDC_IP));
                    return TRUE;
                }
                
                // Basic IP format validation (192.168.x.x)
                int octets[4];
                int valid = (sscanf(p, "%d.%d.%d.%d", &octets[0], &octets[1], &octets[2], &octets[3]) == 4);
                
                if (valid) {
                    for (int i = 0; i < 4; i++) {
                        if (octets[i] < 0 || octets[i] > 255) {
                            valid = 0;
                            break;
                        }
                    }
                }
                
                if (!valid) {
                    MessageBox(hwnd, 
                        "Please enter a valid IP address (format: 192.168.x.x)\n"
                        "Example: 192.168.1.100 or 127.0.0.1",
                        "Invalid IP Address", 
                        MB_OK | MB_ICONERROR);
                    SetFocus(GetDlgItem(hwnd, IDC_IP));
                    return TRUE;
                }
                
                strcpy(server_ip, p);
                EndDialog(hwnd, IDOK);
            } 
            else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwnd, IDCANCEL);
            }
            return TRUE;
    }
    return FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
			// Create fonts
			HDC hdc = GetDC(hWnd);
			int dpi = GetDeviceCaps(hdc, LOGPIXELSY); // обычно 96
			int fontHeight8pt = -MulDiv(8, dpi, 72);  // для 8 pt
			int fontHeight9pt = -MulDiv(9, dpi, 72);  // для 9 pt

			HFONT hFontTahoma = CreateFont(
				fontHeight9pt, 0, 0, 0, FW_NORMAL,
				FALSE, FALSE, FALSE, DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
				"Tahoma"
			);
			
			HFONT hFontTahomaBold = CreateFont(
				fontHeight8pt, 0, 0, 0, FW_BOLD,
				FALSE, FALSE, FALSE, DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
				"Tahoma"
			);

			HFONT hFontLucida = CreateFont(
				fontHeight9pt, 0, 0, 0, FW_NORMAL,
				FALSE, FALSE, FALSE, DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
				"Lucida Console"
			);
			
            // Create UI elements with scrollbars
            hUsersList = CreateWindow("LISTBOX", "", 
                WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
                10, 10, 150, 300, hWnd, (HMENU)ID_USERS, NULL, NULL);
            
			hMsgDisplay = CreateWindow("EDIT", "", 
				WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | 
				ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
				170, 10, 410, 300, hWnd, (HMENU)ID_MSG_DISPLAY, NULL, NULL);
            
            hEdit = CreateWindow("EDIT", "", 
				WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_WANTRETURN | ES_AUTOHSCROLL | WS_VSCROLL,
				170, 320, 350, 60, hWnd, (HMENU)ID_EDIT, NULL, NULL);
            
            hSendBtn = CreateWindow("BUTTON", "Send", 
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                530, 320, 50, 30, hWnd, (HMENU)ID_SEND, NULL, NULL);
			
			// Set Tahoma font for all controls
			SendMessage(hUsersList, WM_SETFONT, (WPARAM)hFontTahomaBold, TRUE);
			SendMessage(hMsgDisplay, WM_SETFONT, (WPARAM)hFontTahoma, TRUE);
			SendMessage(hEdit, WM_SETFONT, (WPARAM)hFontLucida, TRUE);
			SendMessage(hSendBtn, WM_SETFONT, (WPARAM)hFontTahomaBold, TRUE);
            
            // Add users to list
            char user_info[128];
			sprintf(user_info, "%s (You)", computer_name);
			SendMessage(hUsersList, LB_ADDSTRING, 0, (LPARAM)user_info);
			
			if (peer_name[0] != '\0') {
				sprintf(user_info, "%s", peer_name);
				SendMessage(hUsersList, LB_ADDSTRING, 0, (LPARAM)user_info);
			}
			
			g_oldEditProc = (WNDPROC)SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)EditProc);
			return 0;
        }
        
        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_SEND) {
                SendCurrentMessage(hWnd);
            }
            else if (LOWORD(wParam) == IDM_EXIT) {
				if (MessageBox(hWnd,
					"Are you sure to end current session?\nNote: if you want to save chat history, click \"Chat\" menu and then Save.",
					"Messenger",
					MB_ICONQUESTION | MB_YESNO) == IDYES)
				{
					char leave_msg[128];
					sprintf(leave_msg, "[SYSTEM]: %s left the chat", computer_name);
					send(client_socket, leave_msg, strlen(leave_msg), 0);
					AddMessage(leave_msg);
					PlayNotificationSoundLeft();
					if (is_server) LogMessage(leave_msg);
					CleanupAndExit(hWnd, false);
				}
			}
			else if (LOWORD(wParam) == IDM_RESTART) {
				if (MessageBox(hWnd,
					"Are you sure to restart current session?\nNote: if you want to save chat history, click \"Chat\" menu and then Save.",
					"Messenger",
					MB_ICONQUESTION | MB_YESNO) == IDYES)
				{
					char leave_msg[128];
					sprintf(leave_msg, "[SYSTEM]: %s left the chat", computer_name);
					send(client_socket, leave_msg, strlen(leave_msg), 0);
					AddMessage(leave_msg);
					PlayNotificationSoundLeft();
					if (is_server) LogMessage(leave_msg);
					CleanupAndExit(hWnd, true);
				}
			}
			else if (LOWORD(wParam) == IDM_ABOUT) {
				MessageBox(hWnd, "Simple P2P Messenger\n(c) WinXP655, 2025\nCreated with clean Windows API!", "About Messenger", MB_OK | MB_ICONINFORMATION);
			} else if (LOWORD(wParam) == IDM_CLEAR) {
				SetWindowText(hMsgDisplay, "");
			} else if (LOWORD(wParam) == IDM_SAVE) {
				// Generate timestamped filename
				char filename[MAX_PATH];
				time_t now = time(NULL);
				struct tm *tm_info = localtime(&now);
				
				strftime(filename, sizeof(filename), "Chat-%Y%m%d-%H%M%S.txt", tm_info);
				
				// Get the chat text
				int len = GetWindowTextLength(hMsgDisplay);
				char *chatText = (char*)malloc(len + 1);
				GetWindowText(hMsgDisplay, chatText, len + 1);
				
				// Save to file
				FILE *f = fopen(filename, "w");
				if (f) {
					fwrite(chatText, 1, len, f);
					fclose(f);
					
					char msg[256];
					sprintf(msg, "[SYSTEM] Chat saved to %s", filename);
					AddMessage(msg);
					if (is_server) LogMessage(msg);
					
					MessageBox(hWnd, msg, "Chat Saved", MB_OK | MB_ICONINFORMATION);
				} else {
					MessageBox(hWnd, "Failed to save chat file", "Error", MB_OK | MB_ICONERROR);
				}
				
				free(chatText);
			}
            return 0;
        }
		
		case WM_CLOSE:
			if (MessageBox(hWnd,
				"Are you sure to end current session?",
				"Messenger",
				MB_ICONQUESTION | MB_YESNO) == IDYES)
			{
				char leave_msg[128];
				sprintf(leave_msg, "[SYSTEM]: %s left the chat", computer_name);
				send(client_socket, leave_msg, strlen(leave_msg), 0);
				AddMessage(leave_msg);
				PlayNotificationSoundLeft();
				if (is_server) LogMessage(leave_msg);
				CleanupAndExit(hWnd, false);
			}
			DestroyWindow(hWnd);
			return 0;
			
        case WM_DESTROY: {
            if (g_oldEditProc) {
        SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)g_oldEditProc);
    }
    CleanupAndExit(hWnd, false);
    return 0;
        }
		
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC:
            return (LRESULT)GetStockObject(WHITE_BRUSH);
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void CleanupAndExit(HWND hWnd, bool restart) {
    is_running = 0;
    
    // Give receive thread a moment to exit
    Sleep(100);
    
    if (client_socket != INVALID_SOCKET) {
        // Graceful shutdown
        shutdown(client_socket, SD_BOTH);
        
        // Set linger to 0 to force close
        struct linger linger_opt = { 1, 0 }; // Linger active, timeout 0
        setsockopt(client_socket, SOL_SOCKET, SO_LINGER, (char*)&linger_opt, sizeof(linger_opt));
        
        closesocket(client_socket);
        client_socket = INVALID_SOCKET;
    }
    
    if (chatlog != NULL) {
        fprintf(chatlog, "=== Session Ended ===\n\n");
        fclose(chatlog);
        chatlog = NULL;
    }
    
    WSACleanup();
    
    if (restart) {
        char path[MAX_PATH];
        GetModuleFileName(NULL, path, MAX_PATH);
        ShellExecute(NULL, "open", path, NULL, NULL, SW_SHOW);
    }
    
    PostQuitMessage(0);
}

void InitializeNetwork(bool server_mode) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(1,1), &wsa);

    if (server_mode) {
        SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server_addr = {0};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);

        bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
        listen(server_fd, 1);
        
        char hostname[256];
        gethostname(hostname, sizeof(hostname));
        struct hostent* host = gethostbyname(hostname);
        strcpy(server_ip, inet_ntoa(*(struct in_addr*)host->h_addr_list[0]));
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShowServerIPMessage, NULL, 0, NULL);
        
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);
        client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        closesocket(server_fd);
        
        strcpy(peer_ip, inet_ntoa(client_addr.sin_addr));
        recv(client_socket, peer_name, sizeof(peer_name), 0);
        send(client_socket, computer_name, strlen(computer_name)+1, 0);
		
        // Add server message to log
        char sys_msg[256];
        sprintf(sys_msg, "[SYSTEM] %s connected from %s", peer_name, peer_ip);
        AddMessage(sys_msg);
		PlayNotificationSoundSystem();
        LogMessage(sys_msg);
    } else {
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
		
		struct timeval timeout;
		timeout.tv_sec = 5;  // 5 секунд на подключение
		timeout.tv_usec = 0;
		setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
		
        struct sockaddr_in server_addr = {0};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = inet_addr(server_ip);

        if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
			char error_msg[256];
			int error_code = WSAGetLastError();

			// Customize error message based on the error code
			switch(error_code) {
				case WSAETIMEDOUT:
					snprintf(error_msg, sizeof(error_msg),
							"[SYSTEM] Connection to %s timed out. The server might be unreachable.", server_ip);
					break;
				case WSAECONNREFUSED:
					snprintf(error_msg, sizeof(error_msg),
							"[SYSTEM] Connection to %s was refused. The server may not be running.", server_ip);
					break;
				default:
					snprintf(error_msg, sizeof(error_msg),
							"[SYSTEM] Failed to connect to %s (Error: %d).", server_ip, error_code);
					break;
			}

			MessageBox(NULL, error_msg, "P2P Messenger - Connection Error", MB_OK | MB_ICONERROR);

			closesocket(client_socket);
			WSACleanup();
			exit(1);
		}
		
		timeout.tv_sec = 0;
		setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
		
		struct sockaddr_in server_info;
        int len = sizeof(server_info);
        getsockname(client_socket, (struct sockaddr*)&server_info, &len);
        strcpy(peer_ip, inet_ntoa(server_info.sin_addr));
		
		send(client_socket, computer_name, strlen(computer_name)+1, 0);
        recv(client_socket, peer_name, sizeof(peer_name), 0);

        PlayNotificationSoundJoin();

        // Добавляем системное сообщение
        char sys_msg[256];
        sprintf(sys_msg, "[SYSTEM] Connected to %s at %s", peer_name, server_ip);
        AddMessage(sys_msg);
    }
    
    _beginthread(ReceiveMessages, 0, NULL);
}

void ReceiveMessages(void* arg) {
    char buffer[BUFFER_SIZE];
    
    while (is_running) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAETIMEDOUT) {
                continue;  // Пропускаем таймауты, не разрывая соединение
            }
            
            // Реальная ошибка соединения
            char msg[128] = "[SYSTEM] Connection with server or client lost";
            AddMessage(msg);
            if (is_server) LogMessage(msg);
            is_running = 0;
            break;
        }
        else if (bytes == 0) {
            // Graceful shutdown от сервера
            char msg[128] = "[SYSTEM] Server has closed the connection";
            AddMessage(msg);
            if (is_server) LogMessage(msg);
            is_running = 0;
            break;
        }
        
        // Play sound when receiving a new message (except system messages)
        if (strstr(buffer, "[SYSTEM]") == NULL) {
            PlayNotificationSound();
        } else {
            PlayNotificationSoundSystem();
        }
        
        AddMessage(buffer);
		FlashWindowOnMessage(hwnd_global);
        if (is_server) LogMessage(buffer);
    }
    _endthread();
}

void SendCurrentMessage(HWND hWnd) {
    char buffer[BUFFER_SIZE];
    GetWindowText(hEdit, buffer, BUFFER_SIZE);
    
    // Trim whitespace and newlines from both ends
    char* start = buffer;
    while (isspace(*start)) start++;
    
    char* end = buffer + strlen(buffer) - 1;
    while (end >= start && isspace(*end)) {
        *end-- = '\0';
    }
    
    if (strlen(start) > 0) {  // Only send if non-empty after trimming
        char full_msg[BUFFER_SIZE + 128];
        sprintf(full_msg, "[%s]: %s", computer_name, start);
        send(client_socket, full_msg, strlen(full_msg), 0);
        AddMessage(full_msg);
        if (is_server) LogMessage(full_msg);
    }
    
    // Always clear the input box
    SetWindowText(hEdit, "");
    
    // Remove any lingering Enter key messages
    MSG nextMsg;
    while (PeekMessage(&nextMsg, hWnd, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)) {
        if (nextMsg.message == WM_KEYDOWN && nextMsg.wParam == VK_RETURN)
            continue;
        DispatchMessage(&nextMsg);
    }
}

void AddMessage(const char* msg) {
    // Get current text length
    int len = GetWindowTextLength(hMsgDisplay);
    
    // Append new message
    SendMessage(hMsgDisplay, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    
    // Only add newline if not the first message
    if (len > 0) {
        SendMessage(hMsgDisplay, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
    }
    
    SendMessage(hMsgDisplay, EM_REPLACESEL, FALSE, (LPARAM)msg);
    
    // Auto-scroll to bottom
    SendMessage(hMsgDisplay, WM_VSCROLL, SB_BOTTOM, 0);
}