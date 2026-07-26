#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <process.h> // Для _beginthread()

#define PORT 8888
#define BUFFER_SIZE 1024

SOCKET client_socket = INVALID_SOCKET;
int is_running = 1;

// Функция для приема сообщений
void receive_messages(void* arg) {
    char buffer[BUFFER_SIZE];
    while (is_running) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes <= 0) {
            printf("Connection lost or error occurred\n");
            is_running = 0;
            break;
        }
        printf("\nReceived: %s", buffer);
        printf("> "); // Перерисовываем приглашение для ввода
        fflush(stdout);
    }
    _endthread();
}

int main(int argc, char* argv[]) {
    WSADATA wsa;
    SOCKET server_fd;
    struct sockaddr_in server_addr, client_addr;
    int client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char mode;

    printf("Choose mode (s for server, c for client): ");
    scanf("%c", &mode);
    getchar(); // Очистка буфера

    // Инициализация Winsock
    if (WSAStartup(MAKEWORD(1, 1), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    if (mode == 's') {
        // Режим сервера
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);

        bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
        listen(server_fd, 1);

        printf("Server started. Waiting for connection...\n");
        client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        printf("Client connected: %s\n", inet_ntoa(client_addr.sin_addr));
    }
    else if (mode == 'c') {
        // Режим клиента
        char server_ip[16];
        printf("Enter server IP: ");
        fgets(server_ip, sizeof(server_ip), stdin);
        server_ip[strcspn(server_ip, "\n")] = '\0';

        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = inet_addr(server_ip);

        if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            printf("Connection failed\n");
            closesocket(client_socket);
            WSACleanup();
            return 1;
        }
        printf("Connected to server\n");
    }
    else {
        printf("Invalid mode\n");
        WSACleanup();
        return 1;
    }

    // Запускаем поток для приема сообщений
    _beginthread(receive_messages, 0, NULL);

    // Основной цикл для отправки сообщений
    printf("> ");
    while (is_running) {
        fgets(buffer, BUFFER_SIZE, stdin);
        if (strncmp(buffer, "exit", 4) == 0) {
            is_running = 0;
            break;
        }
        send(client_socket, buffer, strlen(buffer), 0);
        printf("> ");
    }

    // Завершение работы
    closesocket(client_socket);
    if (mode == 's') closesocket(server_fd);
    WSACleanup();
    return 0;
}