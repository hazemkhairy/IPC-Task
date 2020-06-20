
#pragma warning(disable:4996) 
#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <vector>
#include <filesystem>
#include <direct.h>
using namespace std;

WSADATA WSAData;
int main()
{
    cout << "--------------------------------------------------------\n";
    cout << "--------------------------------------------------------\n";
    cout << "----------------------LOGGER----------------------------\n";
    cout << "--------------------------------------------------------\n";
    cout << "--------------------------------------------------------\n";
    int WSok = WSAStartup(MAKEWORD(2, 2), &WSAData);
    if (WSok != 0)
    {
        cout << "COULDN'T OPEN WSA ";
        return 2;
    }
    SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == INVALID_SOCKET)
    {
        cout << "COULDN'T OPEN SOCKET";
        return 2;
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1998);
    addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    
    int be = bind(listener, (sockaddr*)&addr, sizeof(addr));
    if (be !=0)
    {
        cout << "COULDN'T Bind socket  " << WSAGetLastError() ;
        return 2;
    }
    int le = listen(listener, SOMAXCONN);
    if (le != 0)
    {
        cout << "COULDN'T LISTEN " << WSAGetLastError();
        return 2;
    }
    int messageID = 1;
    string delimitter = "-----------------------------------------------------------------";
    while (1)
    {
        sockaddr_in senderAddr;
        int senderAddrLen = sizeof(senderAddr);
        SOCKET sender = accept(listener, (sockaddr*)&senderAddr, &senderAddrLen);
        if (sender == INVALID_SOCKET)
        {
            cout << "COULDN't ACEEPT " << WSAGetLastError() << endl;
            continue;
        }
        char buffer[2048] = {};
        int byteRecieved = recv(sender, buffer, sizeof(buffer), 0);
        if (byteRecieved == SOCKET_ERROR)
        {
            cout << "ERROR DURING RECV " << WSAGetLastError() << endl;
            continue;
        }
        else
        {
            cout << delimitter<<'\n';
            cout << "Message #" << messageID++ << endl;
            cout << buffer << endl;
            cout << delimitter << '\n';
        }
        /*for (int i = 0 ; i < byteRecieved;i++)
            cout << buffer[i];
        cout << endl;*/

    }
}