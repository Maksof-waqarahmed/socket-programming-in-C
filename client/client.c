#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Connection failed. Error Code: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Connected to server.\n");
    printf("Type your messages. Type '/send filename.txt' to send a file, '/exit' to quit.\n");

    while (1)
    {
        printf("You (Client): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "/exit") == 0)
        {
            send(sock, buffer, strlen(buffer), 0);
            break;
        }
        else if (strncmp(buffer, "/send ", 6) == 0)
        {
            char *filename = buffer + 6;
            FILE *fp = fopen(filename, "rb");
            if (fp == NULL)
            {
                perror("File open error");
                continue;
            }

            send(sock, buffer, strlen(buffer), 0);
            Sleep(100); // Ensure the command is processed first

            int n;
            while ((n = fread(buffer, 1, BUFFER_SIZE, fp)) > 0)
            {
                send(sock, buffer, n, 0);
            }

            fclose(fp);
            printf("File sent successfully.\n");
        }
        else
        {
            send(sock, buffer, strlen(buffer), 0);

            int bytesReceived = recv(sock, buffer, BUFFER_SIZE - 1, 0);
            if (bytesReceived > 0)
            {
                buffer[bytesReceived] = '\0';

                if (strncmp(buffer, "/send ", 6) == 0)
                {
                    char *filename = buffer + 6;
                    FILE *fp = fopen(filename, "wb");
                    if (fp == NULL)
                    {
                        perror("File open error");
                        continue;
                    }

                    printf("Receiving file '%s' from server...\n", filename);

                    while ((bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0)) > 0)
                    {
                        fwrite(buffer, 1, bytesReceived, fp);
                        if (bytesReceived < BUFFER_SIZE)
                            break;
                    }

                    fclose(fp);
                    printf("File '%s' received and saved.\n", filename);
                }
                else
                {
                    printf("Server: %s\n", buffer);
                }
            }
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
