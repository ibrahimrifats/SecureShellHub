#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>

using namespace std;

string serverPublicKey;

void loadPublicKey() {
    ifstream pubKeyFile("id_rsa.pub");
    if (!pubKeyFile.is_open()) {
        cerr << "Unable to open id_rsa.pub file." << endl;
        exit(1);
    }

    stringstream buffer;
    buffer << pubKeyFile.rdbuf();
    serverPublicKey = buffer.str();
    pubKeyFile.close();
}

bool authenticateClient(const string& clientPublicKey) {
    return clientPublicKey == serverPublicKey;
}

void executeCommand(int clientSocket) {
    char buffer[1024];
    int bytesReceived;

    while (true) {
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            cerr << "Failed to receive command or client disconnected." << endl;
            break;
        }
        buffer[bytesReceived] = '\0';
        string command = buffer;
        if (command == "exit") break;

        FILE* fp = popen(command.c_str(), "r");
        if (!fp) {
            string errorMessage = "Failed to execute command.";
            send(clientSocket, errorMessage.c_str(), errorMessage.length(), 0);
            continue;
        }

        string output;
        while (fgets(buffer, sizeof(buffer), fp)) {
            output += buffer;
        }

        send(clientSocket, output.c_str(), output.length(), 0);
        fclose(fp);
    }

    close(clientSocket);
}

void handleClient(int clientSocket) {
    char buffer[1024];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    buffer[bytesReceived] = '\0';

    string clientPublicKey = buffer;

    if (authenticateClient(clientPublicKey)) {
        string response = "Successfully authenticated.";
        send(clientSocket, response.c_str(), response.length(), 0);
        executeCommand(clientSocket);
    } else {
        string response = "Authentication failed.";
        send(clientSocket, response.c_str(), response.length(), 0);
    }
}

void startServer() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addr_size;

    loadPublicKey();

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cerr << "Socket creation failed." << endl;
        exit(1);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = 8081;
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Binding failed." << endl;
        exit(1);
    }

    if (listen(serverSocket, 10) == 0) {
        cout << "Server listening on port 8080..." << endl;
    } else {
        cerr << "Server failed to listen." << endl;
        exit(1);
    }

    while (true) {
        addr_size = sizeof(clientAddr);
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addr_size);
        if (clientSocket < 0) {
            cerr << "Server failed to accept client." << endl;
            continue;
        }

        cout << "Client connected, spawning thread..." << endl;
        
        // Handle client in a separate thread
        thread clientThread(handleClient, clientSocket);
        clientThread.detach(); // Detach the thread to allow it to run independently
    }

    close(serverSocket);
}

int main() {
    startServer();
    return 0;
}
