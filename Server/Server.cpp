#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>

#pragma warning(disable: 4996)

using namespace std;

int recvAll(SOCKET socket, char* buffer, int totalBytes) {
    int bytesReceived = 0;
    while (bytesReceived < totalBytes) {
        int result = recv(socket, buffer + bytesReceived, totalBytes - bytesReceived, 0);
        if (result <= 0) {
            return -1;
        }
        bytesReceived += result;
    }
    return bytesReceived;
}

int main()
{
    setlocale(LC_ALL, "Rus");
    WSAData wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Error\n";
        exit(1);
    }
    
    SOCKADDR_IN addr;
    int sizeAddr = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(1111);
    addr.sin_family = AF_INET;

    SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
    if (sListen == INVALID_SOCKET) {
        cout << "Ошибка создания сокета\n";
        WSACleanup();
        return 1;
    }

    bind(sListen, (SOCKADDR*)&addr, sizeAddr);
    listen(sListen, SOMAXCONN);

    cout << "Waiting for connection...\n";

    SOCKET newConn;
    newConn = accept(sListen, (SOCKADDR*)&addr, &sizeAddr);

    if (newConn == INVALID_SOCKET) {
        cout << "Ошибка подключения\n";
        closesocket(sListen);
        WSACleanup();
        return 1;
    }
    cout << "Клиент подключен\n";

    int fileNameLength;
    int bytesReceived = recv(newConn, (char*)&fileNameLength, sizeof(int), 0);
    if (bytesReceived != sizeof(int)) {
        cout << "Ошибка: не удалось получить длину имени файла\n";
        closesocket(newConn);
        closesocket(sListen);
        WSACleanup();
        return 1;
    }
    cout << "Длина имени файла: " << fileNameLength << " байт" << endl;

    vector<char> fileName(fileNameLength + 1, 0); // +1 для '\0'
    if (recvAll(newConn, fileName.data(), fileNameLength) == -1) {
        cout << "Ошибка: не удалось получить имя файла\n";
        closesocket(newConn);
        closesocket(sListen);
        WSACleanup();
        return 1;
    }
    string fileNameStr(fileName.data());
    cout << "Имя файла: " << fileNameStr << endl;

    int fileSize;
    bytesReceived = recv(newConn, (char*)&fileSize, sizeof(int), 0);
    if (bytesReceived != sizeof(int)) {
        cout << "Ошибка: не удалось получить размер файла\n";
        closesocket(newConn);
        closesocket(sListen);
        WSACleanup();
        return 1;
    }
    cout << "Размер файла: " << fileSize << " байт\n";

    vector<char> fileData(fileSize);
    int totalReceived = 0;
    cout << "Приём данных...\n";

    while (totalReceived < fileSize) {
        int bytesRead = recv(newConn, fileData.data() + totalReceived, fileSize - totalReceived, 0);
        if (bytesRead <= 0) {
            cout << "Ошибка при приёме файла\n";
            closesocket(newConn);
            closesocket(sListen);
            WSACleanup();
            return 1;
        }
        totalReceived += bytesRead;

        cout << "Прогресс: " << totalReceived << "/" << fileSize << " байт ("
            << (totalReceived * 100 / fileSize) << "%)\r";
    }
    cout << "\nФайл полностью принят!\n";

    string savePath = "received_" + fileNameStr;
    ofstream outputFile(savePath, ios::binary);
    if (!outputFile) {
        cout << "Ошибка: не удалось создать файл на диске\n";
        closesocket(newConn);
        closesocket(sListen);
        WSACleanup();
        return 1;
    }

    outputFile.write(fileData.data(), fileSize);
    outputFile.close();
    cout << "Файл сохранён как: " << savePath << endl;

    string response = "Файл успешно получен и сохранён! Размер: " + to_string(fileSize) + " байт";
    send(newConn, response.c_str(), response.length(), 0);
    cout << "\nОтправлено подтверждение клиенту\n";

    cout << "\nЗакрываем соединения...\n";
    closesocket(newConn);
    closesocket(sListen);
    WSACleanup();

    system("pause");
}