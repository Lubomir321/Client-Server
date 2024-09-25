#include <iostream>
#include <vector>
#include <algorithm>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <omp.h>
#include <thread>
#include <mutex>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

// Function to perform quick sort
void quickSort(int arr[], int low, int high) {
    if (low < high) {
        int pivot = arr[high];
        int i = low - 1;

        for (int j = low; j <= high - 1; j++) {
            if (arr[j] < pivot) {
                i++;
                swap(arr[i], arr[j]);
            }
        }
        swap(arr[i + 1], arr[high]);
        int pi = i + 1;

        // Parallelize quicksort on both halves of the array
#pragma omp parallel sections
        {
#pragma omp section
            {
                quickSort(arr, low, pi - 1);
            }
#pragma omp section
            {
                quickSort(arr, pi + 1, high);
            }
        }
    }
}

// Function to handle client connections
void handleClient(SOCKET clientSocket) {
    // Receive array from client
    int array[100]; // Assuming max array size is 100
    int recvResult = recv(clientSocket, reinterpret_cast<char*>(array), sizeof(array), 0);
    if (recvResult == SOCKET_ERROR) {
        cerr << "Receive failed: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        return;
    }

    // Determine array size
    int arraySize = recvResult / sizeof(int);

    // Sort the array
    quickSort(array, 0, arraySize - 1);

    // Send sorted array back to client
    send(clientSocket, reinterpret_cast<char*>(array), arraySize * sizeof(int), 0);

    // Cleanup
    closesocket(clientSocket);
}

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cerr << "WSAStartup failed: " << result << endl;
        return 1;
    }

    // Create a socket
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        cerr << "Error creating socket: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // Bind the socket
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(5000); // Port to listen on

    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed with error: " << WSAGetLastError() << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Listen failed with error: " << WSAGetLastError() << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server started. Waiting for connections..." << endl;

    //// Accept incoming client connections in a loop
    //while (true) {
    //    SOCKET clientSocket = accept(listenSocket, NULL, NULL);
    //    if (clientSocket == INVALID_SOCKET) {
    //        cerr << "Accept failed: " << WSAGetLastError() << endl;
    //        closesocket(listenSocket);
    //        WSACleanup();
    //        return 1;
    //    }

    //    cout << "Connection established with client." << endl;

    //    // Handle client connection in a separate thread
    //    thread clientThread(handleClient, clientSocket);
    //    clientThread.detach(); // Detach the thread to let it run independently
    //}

    //from exercise 2024-04-09
    fd_set masterSet, readSet;
    FD_ZERO(&masterSet);
    FD_ZERO(&readSet);
    FD_SET(listenSocket, &masterSet);
    int maxSd = listenSocket;

    // Main loop
    while (true) {
        readSet = masterSet;

        int activity = select(maxSd + 1, &readSet, nullptr, nullptr, nullptr);

        if (activity == SOCKET_ERROR) {
            cerr << "select call failed: " << WSAGetLastError() << endl;
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }

        // Check each socket for activity
        for (int i = 0; i <= maxSd; i++) {
            if (FD_ISSET(i, &readSet)) {
                if (i == listenSocket) {
                    // New connection
                    SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
                    if (clientSocket == INVALID_SOCKET) {
                        cerr << "Accept failed: " << WSAGetLastError() << endl;
                    }
                    else {
                        cout << "New connection established." << endl;
                        FD_SET(clientSocket, &masterSet);
                        if (clientSocket > maxSd) {
                            maxSd = clientSocket;
                        }
                    }
                }
                else {
                    // Handle data from an existing connection
                    handleClient(i);
                    FD_CLR(i, &masterSet);
                }
            }
        }
    }


    closesocket(listenSocket);
    WSACleanup();
    return 0;
}



