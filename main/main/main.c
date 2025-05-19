#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 12345
#define BUF_SIZE 1024

SOCKET conn;
int running = 1;

DWORD WINAPI recv_thread(LPVOID lpParam) {
    char buffer[BUF_SIZE];
    int len;

    while (running) {
        len = recv(conn, buffer, BUF_SIZE - 1, 0);
        if (len <= 0) {
            printf("\n[상대방 연결 종료]\n");
            running = 0;
            break;
        }
        buffer[len] = '\0';

        printf("\r상대방: %s", buffer);
        printf("나: ");
        fflush(stdout);
    }
    return 0;
}

void error_exit(const char* msg) {
    fprintf(stderr, "%s (에러 코드: %d)\n", msg, WSAGetLastError());
    exit(1);
}

int main() {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in addr, client_addr;
    int mode;
    char buffer[BUF_SIZE];
    char ip[100];
    int addrlen = sizeof(client_addr);
    HANDLE thread;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        error_exit("WSAStartup 실패");
    }

    printf("모드 선택: 1) 서버  2) 클라이언트\n입력: ");
    scanf("%d", &mode);
    getchar();

    if (mode == 1) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) error_exit("소켓 생성 실패");

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(PORT);

        if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
            error_exit("bind 실패");

        if (listen(sock, 1) == SOCKET_ERROR)
            error_exit("listen 실패");

        printf("상대방의 연결을 기다리는 중입니다...\n");

        conn = accept(sock, (struct sockaddr*)&client_addr, &addrlen);
        if (conn == INVALID_SOCKET)
            error_exit("accept 실패");

        printf("상대방이 연결되었습니다.\n");
    }
    else if (mode == 2) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) error_exit("소켓 생성 실패");

        printf("상대방 IP를 입력하세요: ");
        scanf("%s", ip);
        getchar();

        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT);
        addr.sin_addr.s_addr = inet_addr(ip);

        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
            error_exit("서버에 연결 실패");

        conn = sock;
        printf("서버에 연결되었습니다.\n");
    }
    else {
        printf("잘못된 선택입니다.\n");
        WSACleanup();
        return 1;
    }

    thread = CreateThread(NULL, 0, recv_thread, NULL, 0, NULL);

    while (running) {
        printf("나: ");
        if (fgets(buffer, BUF_SIZE, stdin) != NULL) {
            send(conn, buffer, strlen(buffer), 0);
        }
    }

    closesocket(conn);
    if (mode == 1) closesocket(sock);
    WSACleanup();
    return 0;
}