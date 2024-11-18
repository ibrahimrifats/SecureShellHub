#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

void loadPublicKey() {
    ifstream pubKeyFile("id_rsa.pub");
    if (!pubKeyFile.is_open()) {
        cerr << "Unable to open id_rsa.pub file." << endl;
        exit(1);
    }

    stringstream buffer;
    buffer << pubKeyFile.rdbuf();
    pubKeyFile.close();
}

string generateHexKey() {
    ifstream keyFile("security_key.txt");
    stringstream buffer;
    buffer << keyFile.rdbuf();
    return buffer.str();
}

void connectToServer(const string& hexKey) {
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[1024];

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        cerr << "Connection failed." << endl;
        exit(1);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = 8081;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Connection failed." << endl;
        exit(1);
    }

    send(clientSocket, hexKey.c_str(), hexKey.length(), 0);

    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    buffer[bytesReceived] = '\0';
    cout << "Server Response: " << buffer << endl;

    if (string(buffer) == "Successfully authenticated.") {
        while (true) {
            cout << "Enter command: ";
            string cmd;
            getline(cin, cmd);
            if (cmd == "exit") break;

            send(clientSocket, cmd.c_str(), cmd.length(), 0);

            bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            buffer[bytesReceived] = '\0';
            cout << "Output: " << buffer << endl;
        }
    }

    close(clientSocket);
}

int main() {
    loadPublicKey();
    string hexKey = generateHexKey();
    connectToServer(hexKey);
    return 0;
}
