#define _CRT_SECURE_NO_WARNINGS 

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>

#pragma warning(disable: 4996)

using namespace std;

enum Role {
    ADMIN,
    USER
};

enum Status {
    ACTIVE,
    INACTIVE
};

enum Action {
    LOGIN,
    CREATE,
    EDIT,
    REMOVE,
    PRINTALLUSERS,
    LOGOUT
};

class User {
    string name;
    Role role;
    Status status;
public:
    User(string _name, Role _role, Status _status) : name(_name), role(_role), status(_status) {}

    User(string data) {
        loadFromString(data);
    }
   
    string toString();

    void loadFromString(string data);

    friend bool operator == (const User& user1, const User& user2) {
        return (user1.name == user2.name);
    }

    string getName() {
        return name;
    }

    void setName(string _name) {
        if(!_name.empty())
            name = _name;
    }

    Role getRole() {
        return role;
    }

    void setRole(Role _role) {
        role = _role;
    }

    Status getStatus() {
        return status;
    }

    void setStatus(Status _status) {
        status = _status;
    }
};

class UserStorage {
    vector<User> users;

public:
    void saveUsers(string filename);
    void loadUsers(string filename);

    void addUser(User user);
    User* findUser(string name);
    void removeUser(User* user);

    bool checkName(string name);

    void printAllUsers();
};

class LogEntry {
    time_t timestamp;
    string actor;
    Action action;
    string target;
    string details;
public:
    LogEntry(string _actor, Action _action) : actor(_actor), action(_action) {
        time(&timestamp);
    }

    LogEntry(string _actor, Action _action, string _target, string _details) : actor(_actor), action(_action), target(_target), details(_details) {
        time(&timestamp);
    }

    string toString();
};

class Session {
    User user;
    string logFilename;
    vector<LogEntry> logs;
public:
    Session(User _user, string _logFilename) : user(_user), logFilename(_logFilename) {}

    User getUser() {
        return user;
    }

    void addLog(LogEntry log) {
        logs.push_back(log);
    }

    void saveLogfile() {
        ofstream file(logFilename);
        if (!file.is_open()) {
            throw runtime_error("File cannot be saved\n");
        }
        for (auto& l : logs) {
            file << l.toString() << '\n';
        }
        file.close();
    }
};

string LogEntry::toString() {
    string s = "";
    string time = ctime(&timestamp);
    s += time.substr(0, time.size() - 1);
    s += " | ";
    s += actor;
    s += " | ";
    switch (action) {
    case LOGIN:
        s += "login";
        break;
    case CREATE:
        s += "create";
        break;
    case EDIT:
        s += "edit";
        break;
    case REMOVE:
        s += "remove";
        break;
    case LOGOUT:
        s += "logout";
        break;
    case PRINTALLUSERS:
        s += "print_all_users";
        break;
    }
    if (!target.empty()) {
        s += " | ";
        s += target;
    }
    if (!details.empty()) {
        s += " | ";
        s += details;
    }
    return s;
}

void UserStorage::saveUsers(string filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("File cannot be saved\n");
    }
    for (auto& user : users) {
        file << user.toString() << '\n';
    }
    file.close();
    cout << "Пользователи сохранены в файл " << filename << endl;
}

void UserStorage::loadUsers(string filename) {
    users.clear();
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("File cannot be opened\n");
    }
    string line;
    
    while (getline(file, line)) {
        User user(line);
        users.push_back(user);
    }
    file.close();
    cout << "Пользователи загружены из файла " << filename << endl;
}

void UserStorage::addUser(User user) {
    for (auto& u : users) {
        if (u == user) {
            throw runtime_error("Такой пользователь уже есть\n");
        }
    }
    users.push_back(user);
    cout << "Добавлен пользователь " << user.getName() << endl;
    saveUsers("user.txt");
}

User* UserStorage::findUser(string name) {
    for (auto& user : users) {
        if (user.getName() == name)
            return &user;
    }
    return nullptr;
}

void UserStorage::removeUser(User* user) {
    if (!user) {
        throw runtime_error("Такого пользователя не существует\n");
    }
        
    users.erase(remove(users.begin(), users.end(), *user), users.end());
    cout << "Пользователь " << user->getName() << " удален\n";
    saveUsers("user.txt");
}

bool UserStorage::checkName(string name) {
    for (auto& u : users) {
        if (u.getName() == name)
            return false;
    }
    return true;
}

void UserStorage::printAllUsers() {
    for (auto& u : users) {
        cout << u.toString() << '\n';
    }
    cout << endl;
}

string User::toString() {
    string user = "";
    user += name;
    user += ';';
    string _role, _status;
    if (role == ADMIN)
        _role = "admin;";
    else if (role == USER)
        _role = "user;";
    user += _role;

    if (status == ACTIVE)
        _status = "active;";
    else if (status == INACTIVE)
        _status = "inactive;";
    user += _status;

    return user;
}

void User::loadFromString(string data) {
    int first = data.find(';');
    int second = data.find(';', first + 1);
    int third = data.find(';', second + 1);

    string _name = data.substr(0, first);
    string _role = data.substr(first + 1, second - first - 1);
    string _status = data.substr(second + 1, third - second - 1);

    name = _name;

    if (_role == "admin")
        role = ADMIN;
    else if (_role == "user")
        role = USER;

    if (_status == "active")
        status = ACTIVE;
    else if (_status == "inactive")
        status = INACTIVE;
}

void adminMenu(UserStorage& storage, Session& session) {
    User admin = session.getUser();
    int actions = 0;
    while (true) {
        if (actions % 2 == 0)
            session.saveLogfile();
        cout << "Меню администратора:\n1.Добавить пользователя\n2.Изменить пользователя\n3.Удалить пользователя\n4.Показать всех пользователей\n5.Выход\nВыберите действие: ";
        int ch;
        cin >> ch;
        cin.ignore();
        switch (ch) {
        case 1: {
            string user;
            cout << "Введите имя нового пользователя (оно должно быть уникальным): ";
            string name;
            getline(cin, name);
            user += name + ';';
            cout << "Введите роль нового пользователя (user\\admin): ";
            string role;
            getline(cin, role);
            user += role + ";active;";
            User target(user);
            storage.addUser(target);
            LogEntry add(admin.getName(), CREATE, target.getName(), "");
            session.addLog(add);
            actions++;
            break;
        }
        case 2: {
            cout << "Введите имя пользователя для редактирования: ";
            string name;
            getline(cin, name);
            User* target = storage.findUser(name);
            if (!target) {
                cout << "Пользователь не найден\n";
                break;
            }
            int ch;
            string details = "";
            cout << "Иеню действий:\n1.Изменить имя пользователя\n2.Изменить роль пользователя\n3.Изменить статус пользователя\n4.Выйти в прошлое меню\nВыберите действие: ";
            cin >> ch;
            cin.ignore();
            switch (ch) {
            case 1: {
                string newName;
                cout << "Введите новое имя пользователя: ";
                getline(cin, newName);
                if (storage.checkName(name)) {
                    details += "Изменение имени: " + target->getName() + " -> " + newName;
                    target->setName(newName);
                    break;
                }
                else {
                    cout << "Неудача, пользователь с таким именем уже есть\n";
                    details += "Неудачное Изменение имени: " + target->getName() + " -> " + newName + ". Пользователь с таким именем уже есть.";
                    break;
                }
            }
            case 2: {
                Role role = target->getRole();
                string tarRole;
                if (role == ADMIN)
                    tarRole = "Admin";
                else if (role == USER)
                    tarRole = "User";
                int chRole;
                cout << "1.Админ\n2.Пользователь\nВыберите: ";
                cin >> chRole;
                if (chRole == 1 && tarRole != "Admin") {
                    details += "Изменение роли: " + tarRole + " -> Admin";
                    target->setRole(ADMIN);
                    break;
                }
                else if(chRole == 2 && tarRole != "User") {
                    details += "Изменение роли: " + tarRole + " -> User";
                    target->setRole(USER);
                    break;
                }
                else {
                    cout << "Неудача, некорректная роль\n";
                    details += "Неудачное изменение роли: пользователю уже присвоена эта роль.";
                    break;
                }
            }
            case 3: {
                Status status = target->getStatus();
                string tarStatus;
                if (status == ACTIVE)
                    tarStatus = "Active";
                else if (status == INACTIVE)
                    tarStatus = "Inactive";
                int chRole;
                cout << "1.Активный\n2.Неактивный\nВыберите: ";
                cin >> chRole;
                if (chRole == 1 && tarStatus != "Active") {
                    details += "Изменение статуса: " + tarStatus + " -> Active";
                    target->setStatus(ACTIVE);
                    break;
                }
                else if (chRole == 2 && tarStatus != "Inactive") {
                    details += "Изменение статуса: " + tarStatus + " -> Inactive";
                    target->setStatus(INACTIVE);
                    break;
                }
                else {
                    cout << "Неудача, некорректный статус\n";
                    details += "Неудачное изменение статуса: пользователю уже присвоен этот статус.";
                    break;
                }
            }
            case 4: {
                details += "Отмена редактирования.";
                break;
            }
            default:
                details += "Выбраное неправильнон действие.";
                cout << "Неправильное действие\n";
                break;
            }
            LogEntry edit(admin.getName(), EDIT, target->getName(), details);
            session.addLog(edit);
            actions++;
            break;
        }
        case 3: {
            cout << "Введите имя пользователя для удаления: ";
            string name;
            getline(cin, name);
            User* target = storage.findUser(name);
            if (!target) {
                cout << "Пользователь не найден\n";
                break;
            }
            LogEntry remove(admin.getName(), REMOVE, target->getName(), "");
            storage.removeUser(target);
            session.addLog(remove);
            actions++;
            break;
        }
        case 4: {
            storage.printAllUsers();
            LogEntry print(admin.getName(), PRINTALLUSERS);
            session.addLog(print);
            actions++;
            break;
        }
        case 5: {
            cout << "Сессия окончена\n";
            LogEntry logout(admin.getName(), LOGOUT);
            session.addLog(logout);
            session.saveLogfile();
            return;
        }
        default:
            cout << "Неправильное действие\n";
        }
    }
}

int sendAll(SOCKET socket, const char* data, int totalBytes) {
    int bytesSent = 0;
    while (bytesSent < totalBytes) {
        int result = send(socket, data + bytesSent, totalBytes - bytesSent, 0);
        if (result == SOCKET_ERROR) {
            return -1;
        }
        bytesSent += result;
    }
    return bytesSent;
}

int main()
{
    setlocale(LC_ALL, "Rus");
    UserStorage storage;
    string filePath = "";
    try {
        storage.loadUsers("user.txt");
        while (true) {
            cout << "Введите имя админа: ";
            string name;
            getline(cin, name);
            User* curr = storage.findUser(name);
            if (curr) {
                if (curr->getRole() == ADMIN) {
                    string log = curr->getName() + "Log.txt";
                    Session session(*curr, log);
                    LogEntry login(curr->getName(), LOGIN);
                    session.addLog(login);
                    adminMenu(storage, session);
                    filePath = log;
                    break;
                }
                else {
                    cout << "Этот пользователь не является администратором\n" << endl;
                    continue;
                }
            }
            else {
                cout << "Такой пользователь не найден\n" << endl;
                continue;
            }
        }

        cout << endl;

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
        
        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, NULL);
        if (clientSocket == INVALID_SOCKET) {
            cout << "Ошибка создания сокета\n";
            WSACleanup();
            return 1;
        }

        if (connect(clientSocket, (SOCKADDR*)&addr, sizeAddr) == SOCKET_ERROR) {
            cout << "Ошибка подключения. Код: " << WSAGetLastError() << endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
        cout << "Connection succesful\n";


        cout << "Файл для отправки: " << filePath << endl;

        ifstream inputFile(filePath, ios::binary);
        if (!inputFile) {
            cout << "ОШИБКА: Файл " << filePath << " не найден!\n";
            closesocket(clientSocket);
            WSACleanup();
            system("pause");
            return 1;
        }
        cout << "Файл открыт\n";

        // получение размера файла
        inputFile.seekg(0, ios::end);     
        int fileSize = (int)inputFile.tellg(); 
        inputFile.seekg(0, ios::beg);
        cout << "Размер файла: " << fileSize << " байт\n";

        vector<char> fileData(fileSize);
        inputFile.read(fileData.data(), fileSize);
        inputFile.close();
        cout << "Файл прочитан в память\n";

        // [4 байта длина имени] [имя файла] [4 байта размер] [данные]

        string fileName = filePath;
        int fileNameLength = (int)fileName.length();

        cout << "\nОтправка данных...\n";

        if (sendAll(clientSocket, (char*)&fileNameLength, sizeof(int)) == -1) {
            cout << "Ошибка отправки длины имени файла\n";
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
        cout << "Отправлена длина имени: " << fileNameLength << " байт" << endl;

        if (sendAll(clientSocket, fileName.c_str(), fileNameLength) == -1) {
            cout << "Ошибка отправки имени файла\n";
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
        cout << "Отправлено имя файла: " << fileName << endl;

        if (sendAll(clientSocket, (char*)&fileSize, sizeof(int)) == -1) {
            cout << "Ошибка отправки размера файла\n";
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
        cout << "Отправлен размер: " << fileSize << " байт" << endl;

        int totalSent = 0;
        while (totalSent < fileSize) {
            int bytesToSend = fileSize - totalSent;
            int result = send(clientSocket, fileData.data() + totalSent, bytesToSend, 0);
            if (result == SOCKET_ERROR) {
                cout << "Ошибка отправки данных. Код: " << WSAGetLastError() << endl;
                closesocket(clientSocket);
                WSACleanup();
                return 1;
            }
            totalSent += result;

            cout << "Прогресс: " << totalSent << "/" << fileSize << " байт ("
                << (totalSent * 100 / fileSize) << "%)\r";
        }
        cout << "\nФайл отправлен!\n";

        cout << "\nОжидание ответа от сервера...\n";
        char responseBuffer[1024];
        int bytesReceived = recv(clientSocket, responseBuffer, sizeof(responseBuffer) - 1, 0);
        if (bytesReceived > 0) {
            responseBuffer[bytesReceived] = '\0';
            cout << "Ответ сервера: " << responseBuffer << endl;
        }
        else {
            cout << "Ответ не получен\n";
        }

        cout << "\nЗакрываем соединение...\n";
        closesocket(clientSocket);
        WSACleanup();

        system("pause");
    }
    catch (exception& e) {
        cerr << e.what();
    }
}