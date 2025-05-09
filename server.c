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
    SOCKET server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET)
    {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
    {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // Listen
    if (listen(server_fd, 3) == SOCKET_ERROR)
    {
        printf("Listen failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("Waiting for connection...\n");
    client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    if (client_socket == INVALID_SOCKET)
    {
        printf("Accept failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("Client connected.\n");

    while (1)
    {
        int valread = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (valread <= 0)
            break;
        buffer[valread] = '\0';

        if (strcmp(buffer, "/exit") == 0)
        {
            printf("Client exited the chat.\n");
            break;
        }
        else if (strncmp(buffer, "/send ", 6) == 0)
        {
            // Extract filename
            char *filename = buffer + 6;
            FILE *fp = fopen(filename, "wb");
            if (fp == NULL)
            {
                perror("File open error");
                continue;
            }

            // Now receive the file data
            printf("Receiving file: %s\n", filename);

            // Read data in chunks and write to the file
            while ((valread = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0)
            {
                fwrite(buffer, 1, valread, fp); // Write to the file
                if (valread < BUFFER_SIZE)      // End of file
                    break;
            }

            fclose(fp); // Close the file after writing
            printf("File '%s' received and saved.\n", filename);
        }
        else
        {
            // Normal chat message
            printf("Client: %s\n", buffer);

            // Server reply
            printf("You (Server): ");
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strcspn(buffer, "\n")] = 0;

            if (strcmp(buffer, "/exit") == 0)
            {
                send(client_socket, buffer, strlen(buffer), 0);
                break;
            }
            else
            {
                send(client_socket, buffer, strlen(buffer), 0);
            }
        }
    }

    closesocket(client_socket);
    closesocket(server_fd);
    WSACleanup();
    return 0;
}
