#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#pragma comment(lib, "ws2_32")
#pragma warning(disable:4996)

using namespace std;
#pragma comment(lib, "ws2_32")
#pragma warning(disable:4996)
SOCKET clients[64];
int numClients = 0;
map<int, string> listUser;

int numClients = 0;
void RemoveClient(SOCKET* clients, int* numClients, int i)
{
    // Xoa client khoi mang
    if (i < *numClients - 1)
        clients[i] = clients[*numClients - 1];
    *numClients--;
}

DWORD WINAPI ClientThread(LPVOID lpParam)
{
    SOCKET client = *(SOCKET*)lpParam;
    int ret;
    char buf[256];
    char user[32], pass[32], tmp[32];

    while (1)
    {
        char cmd[32];

        ret = recv(client, buf, sizeof(buf), 0);

        if (ret <= 0)
        {
            RemoveClient(client);

            return 0;
        }

        buf[ret] = 0;

        printf("Du lieu nhan duoc: %s\n", buf);

        sscanf(buf, "%s", cmd);

        char sbuf[256];

        int id;

        sprintf(sbuf, "%s: %s", user, buf + 3);

        if (strcmp(cmd, "all") == 0)
        {
            for (int i = 0; i < numClients; i++)
            {
                char* msg = buf + strlen(cmd) + 1;

                send(clients[i], sbuf, strlen(sbuf), 0);
            }
        }
        else if (strcmp(cmd, "exit") == 0) {
            const char* msg = "Ban da dang xuat.\n";

            send(client, msg, strlen(msg), 0);

            const char* newMsg = "client dang xuat.\n";

            char newBuf[256];

            sprintf(newBuf, "%s: %s", user, newMsg);

            for (int i = 0; i < numClients; i++) {
                if (clients[i] != client) {
                    send(clients[i], newBuf, strlen(newBuf), 0);
                }
            }

            auto client_out = listUser.find(client);

            listUser.erase(client_out);

            RemoveClient(client);

            return 0;
        }
        else if (strcmp(cmd, "list") == 0) {
            string mess = "Danh sach nguoi dang hoat dong: ";

            for (auto itr = listUser.begin(); itr != listUser.end(); ++itr)
            {
                mess += itr->second + " ";
            }

            mess = mess + "\n";

            const char* output = mess.c_str();

            send(client, output, strlen(output), 0);
        }
        else
        {
            id = atoi(cmd);

            for (int i = 0; i < numClients; i++)
                if (clients[i] == id)
                {
                    char* msg = buf + strlen(cmd) + 1;

                    send(clients[i], sbuf, strlen(sbuf), 0);
                }
        }

    }

    closesocket(client);

    WSACleanup();
}


int main()
{
    // Khoi tao thu vien
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    // Tao socket
    SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // Khai bao dia chi server
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);
    bind(listener, (SOCKADDR*)&addr, sizeof(addr));
    listen(listener, 5);
    fd_set fdread;
    int ret;
    SOCKET clients[64];
    int numClients = 0;
    SOCKET connected[64];
    int numConnected = 0;
    char buf[256];
    char cmd[32], id[32], tmp[32];
    while (1)
    {
        FD_ZERO(&fdread);
        FD_SET(listener, &fdread);
        for (int i = 0; i < numClients; i++)
            FD_SET(clients[i], &fdread);
        ret = select(0, &fdread, 0, 0, 0);
        if (ret > 0)
        {
            // Kiem tra su kien ket noi moi
            if (FD_ISSET(listener, &fdread))
            {
                SOCKET client = accept(listener, NULL, NULL);
                printf("Ket noi moi: %d\n", client);
                const char* msg = "Hello client\n";
                send(client, msg, strlen(msg), 0);
                clients[numClients] = client;
                numClients++;
            }
            for (int i = 0; i < numClients; i++)
            {
                if (FD_ISSET(clients[i], &fdread))
                {
                    ret = recv(clients[i], buf, sizeof(buf), 0);
                    if (ret <= 0)
                    {
                        // Xoa client ra khoi mang clients
                        RemoveClient(clients, &numClients, i);
                        i--;
                        continue;
                    }
                    buf[ret] = 0;
                    printf("Du lieu nhan duoc tu %d: %s\n", clients[i], buf);
                    // Kiem tra trang thai dang nhap
                    SOCKET client = clients[i];
                    int j = 0;
                    for (; j < numConnected; j++)
                        if (connected[j] == client) break;
                    if (j == numConnected)
                    {
                        // Chua dang nhap
                        ret = sscanf(buf, "%s %s %s", cmd, id, tmp);
                        if (ret != 2)
                        {
                            const char* msg = "Sai cu phap. Hay nhap lai.\n";
                            send(client, msg, strlen(msg), 0);
                        }
                        else
                        {
                            if (strcmp(cmd, "client_id:") != 0)
                            {
                                const char* msg = "Sai cu phap. Hay nhap lai.\n";
                                send(client, msg, strlen(msg), 0);
                            }
                            else
                            {
                                const char* msg = "Dung cu phap. Nhap tin nhan de chuyen tiep.\n";
                                send(client, msg, strlen(msg), 0);
                                // Them vao mang
                                connected[numConnected] = client;
                                numConnected++;
                            }
                        }
                    }
                    else
                    {
                        // Da dang nhap
                        char sbuf[256];
                        sprintf(sbuf, "%s: %s", id, buf);
                        for (int j = 0; j < numConnected; j++)
                            if (client != connected[j])
                                send(connected[j], sbuf, strlen(sbuf), 0);
                    }
                }
            }
        }
    }
}