#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include<sstream>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cerr << "WSAStartup failed: " << result << endl;
        return 1;
    }

    // Create a socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Error creating socket: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // Connect to server
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5000); // Port to connect
    inet_pton(AF_INET, "192.168.46.1", &serverAddr.sin_addr); // IP address of the server

    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Failed to connect to server: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Send array to server
    /*int array[] = { 4, 2, 7, 1, 9, 3, 5, 8, 6 , 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
    int arraySize = sizeof(array) / sizeof(array[0]);*/

    vector<int> array;
    cout << "Enter integers (type end to stop):" << endl;
    string input;
    while (cin >> input) {
        if (input == "end") break; // Exit loop if input is "end"
        int temp;
        stringstream ss(input);
        if (ss >> temp) {
            array.push_back(temp);
        }
        else {
            cout << "Invalid input. Please enter an integer or 'end' to stop." << endl;
            cin.clear();
            
        }
    }
    cin.clear();

    send(clientSocket, reinterpret_cast<char*>(array.data()), array.size() * sizeof(int), 0);

    // Receive sorted array from server
    recv(clientSocket, reinterpret_cast<char*>(array.data()), array.size() * sizeof(int), 0);

    // Print sorted array
    cout << "Sorted array received from server: ";
    for (int i = 0; i < array.size(); ++i) {
        cout << array[i] << " ";
    }
    cout << endl;

    // Cleanup
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
