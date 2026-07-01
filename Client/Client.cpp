#define _CRT_SECURE_NO_WARNINGS 

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <cctype>
#include <limits>
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>

#undef max

#pragma warning(disable: 4996)

using namespace std;

const unsigned char KEY = 0x55;

enum Role {
    ADMIN,
    USER
};

enum Action {
    LOGIN,
    LOGIN_FAIL,
    CREATE,
    CREATE_FAIL,
    EDIT,
    EDIT_FAIL,
    REMOVE,
    REMOVE_FAIL,
    PRINTALLUSERS,
    LOGOUT
};

class User {
    string name;
    Role role;
    string pass;
public:
    User(string _name, Role _role, string _pass) : name(_name), role(_role), pass(_pass) {}

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

    string getPass() {
        return pass;
    }

    void setPass(string _pass) {
        pass = _pass;
    }
};

class UserStorage {
    vector<User> users;

public:
    int getAdminCount() {
        int count = 0;
        for (auto& u : users) {
            if (u.getRole() == ADMIN) count++;
        }
        return count;
    }

    void saveUsers(string filename);
    void loadUsers(string filename);
    void XORfile(string& filename);

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

    LogEntry(string _actor, Action _action, string _details) : actor(_actor), action(_action), details(_details) {
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
    Session(User& _user, string _logFilename) : user(_user), logFilename(_logFilename) {}

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

string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return tolower(c);
        });
    return s;
}

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
    case LOGIN_FAIL:
        s += "login_fail";
        break;
    case CREATE:
        s += "create";
        break;
    case CREATE_FAIL:
        s += "create_fail";
        break;
    case EDIT:
        s += "edit";
        break;
    case EDIT_FAIL:
        s += "edit_fail";
        break;
    case REMOVE:
        s += "remove";
        break;
    case REMOVE_FAIL:
        s += "remove_fail";
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
    XORfile(filename);
    cout << "Пользователи сохранены и зашифрованы в файл " << filename << endl;
}

void UserStorage::loadUsers(string filename) {
    XORfile(filename);

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
    XORfile(filename);
    cout << "Пользователи загружены из файла " << filename << endl;
}

void UserStorage::XORfile(string& filename) {
    ifstream in(filename);
    if (!in.is_open()) {
        throw runtime_error("Не удалось открыть файл: " + filename);
    }

    string content;
    string line;
    while (getline(in, line)) {
        if (!content.empty()) content += '\n';
        content += line;
    }
    in.close();

    // XOR для каждого символа
    for (char& c : content) {
        c ^= KEY;
    }

    ofstream out(filename);
    if (!out.is_open()) {
        throw runtime_error("Не удалось записать файл: " + filename);
    }
    out << content;
    out.close();
}

void UserStorage::addUser(User user) {
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
    string name = user->getName();
    users.erase(remove(users.begin(), users.end(), *user), users.end());
    cout << "Пользователь " << name << " удален\n";
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
    user += name + ';';
    string _role;
    if (role == ADMIN)
        _role = "admin;";
    else if (role == USER)
        _role = "user;";
    user += _role;

    user += pass + ';';

    return user;
}

void User::loadFromString(string data) {
    int first = data.find(';');
    int second = data.find(';', first + 1);
    int third = data.find(';', second + 1);

    if (first == string::npos || second == string::npos || third == string::npos)
        throw runtime_error("Некорректный формат строки пользователя");

    string _name = data.substr(0, first);
    string _role = data.substr(first + 1, second - first - 1);
    string _pass = data.substr(second + 1, third - second - 1);

    name = _name;

    if (_role == "admin")
        role = ADMIN;
    else if (_role == "user")
        role = USER;

    pass = _pass;
}

void adminMenu(UserStorage& storage, Session& session) {
    User admin = session.getUser();
    int actions = 0;
    while (true) {
        if (actions % 2 == 0)
            session.saveLogfile();
        cout << "\nМеню администратора:\n1.Добавить пользователя\n2.Изменить пользователя\n3.Удалить пользователя\n4.Показать всех пользователей\n5.Выход\nВыберите действие: ";
        int ch;
        cin >> ch;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        cin.ignore();
        switch (ch) {
        case 1: {
            string user;
            cout << "Введите имя нового пользователя (оно должно быть уникальным): ";
            string name;
            getline(cin, name);
            name = toLower(name);
            if (!storage.checkName(name)) {
                cout << "Пользователь с таким именем уже есть\n";
                LogEntry add(admin.getName(), CREATE_FAIL, "Такой пользователь уже существует: " + name);
                session.addLog(add);
                actions++;
                break;
            }
            user += name + ';';

            cout << "Введите роль нового пользователя (user\\admin): ";
            string role;
            getline(cin, role);
            role = toLower(role);
            if (role != "admin" && role != "user") {
                cout << "Неправильная роль\n";
                LogEntry add(admin.getName(), CREATE_FAIL, "Неправильная роль: " + role);
                session.addLog(add);
                actions++;
                break;
            }
            user += role + ';';
            
            string pass;
            cout << "Введите пароль нового пользователя: ";
            getline(cin, pass);
            if (pass.empty()) {
                cout << "Пароль не может быть пустым\n";
                LogEntry add(admin.getName(), CREATE_FAIL, "Неправильный пароль: пароль пустой");
                session.addLog(add);
                actions++;
                break;
            }
            else if (pass.size() < 5) {
                cout << "Пароль не может быть меньше 5 символов\n";
                LogEntry add(admin.getName(), CREATE_FAIL, "Неправильный пароль: пароль меньше 5 символов");
                session.addLog(add);
                actions++;
                break;
            }
            user += pass + ';';

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
            name = toLower(name);
            User* target = storage.findUser(name);
            if (!target) {
                cout << "Пользователь не найден\n";
                LogEntry edit(admin.getName(), EDIT_FAIL, name, "Пользователь не найден");
                session.addLog(edit);
                actions++;
                break;
            }
            if (target->getName() == session.getUser().getName()) {
                cout << "Нельзя изменять самого себя\n";
                LogEntry edit(admin.getName(), EDIT_FAIL, target->getName(), "Попытка изменить самого себя");
                session.addLog(edit);
                actions++;
                break;
            }
            int ch;
            string details = "";
            Action action = EDIT;
            cout << "Меню действий:\n1.Изменить имя пользователя\n2.Изменить роль пользователя\n3.Изменить пароль пользователя\n4.Выйти в прошлое меню\nВыберите действие: ";
            cin >> ch;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            cin.ignore();
            switch (ch) {
            case 1: {
                string newName;
                cout << "Введите новое имя пользователя: ";
                getline(cin, newName);
                if (newName.empty()) {
                    cout << "Имя не должно быть пустым\n";
                    action = EDIT_FAIL;
                    details += "Неудачное изменение имени: пустое имя";
                    break;
                }
                newName = toLower(newName);
                if (storage.checkName(newName)) {
                    details += "Изменение имени: " + target->getName() + " -> " + newName;
                    target->setName(newName);
                    break;
                }
                else {
                    cout << "Неудача, пользователь с таким именем уже есть\n";
                    action = EDIT_FAIL;
                    details += "Неудачное мзменение имени: " + target->getName() + " -> " + newName + ". Пользователь с таким именем уже есть.";
                    break;
                }
            }
            case 2: {
                Role role = target->getRole();
                int chRole;
                cout << "1.Админ\n2.Пользователь\nВыберите: ";
                cin >> chRole;
                if (cin.fail()) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Неудача, некорректная роль\n";
                    action = EDIT_FAIL;
                    details += "Неудачное изменение роли: некорректный выбор.";
                    break;
                }
                if (chRole == 1 && role == USER) {
                    details += "Изменение роли: User -> Admin";
                    target->setRole(ADMIN);
                    break;
                }
                else if(chRole == 2 && role == ADMIN) {
                    if (storage.getAdminCount() <= 1) {
                        cout << "Нельзя понизить последнего администратора!\n";
                        action = EDIT_FAIL;
                        details += "Попытка понизить последнего админа";
                        break;
                    }
                    details += "Изменение роли: Admin -> User";
                    target->setRole(USER);
                    break;
                }
                else {
                    cout << "Эта роль уже присвоена пользователю\n";
                    action = EDIT_FAIL;
                    details += "Неудачное изменение роли: пользователю уже присвоена эта роль.";
                    break;
                }
            }
            case 3: {
                string pass;
                cout << "Введите новый пароль: ";
                getline(cin, pass);
                if (pass.empty()) {
                    cout << "Пароль не может быть пустым\n";
                    action = EDIT_FAIL;
                    details += "Неудачное изменение пароля: пароль пустой";
                    break;
                }
                else if (pass.size() < 5) {
                    cout << "Пароль не может быть меньше 5 символов\n";
                    action = EDIT_FAIL;
                    details += "Неудачное изменение пароля: пароль меньше 5 символов";
                    break;
                }
                details += "Изменение пароля: новый пароль успешно добавлен";
                target->setPass(pass);
                break;
            }
            case 4: {
                details += "Отмена редактирования.";
                break;
            }
            default:
                action = EDIT_FAIL;
                details += "Выбраное неправильнон действие.";
                cout << "Неправильное действие\n";
                break;
            }
            LogEntry edit(admin.getName(), action, target->getName(), details);
            session.addLog(edit);
            actions++;
            break;
        }
        case 3: {
            cout << "Введите имя пользователя для удаления: ";
            string name;
            getline(cin, name);
            name = toLower(name);
            if (name == session.getUser().getName()) {
                cout << "Нельзя удалить самого себя\n";
                LogEntry remove(admin.getName(), REMOVE_FAIL, name, "Попытка удалить самого себя");
                session.addLog(remove);
                actions++;
                break;
            }
            User* target = storage.findUser(name);
            if (!target) {
                cout << "Пользователь не найден\n";
                LogEntry remove(admin.getName(), REMOVE_FAIL, name, "Пользователь не найден");
                session.addLog(remove);
                actions++;
                break;
            }
            if (target->getRole() == ADMIN && storage.getAdminCount() <= 1) {
                cout << "Нельзя удалить последнего администратора!\n";
                LogEntry remove(admin.getName(), REMOVE_FAIL, name, "Попытка удалить последнего админа");
                session.addLog(remove);
                actions++;
                break;
            }
            storage.removeUser(target);
            LogEntry remove(admin.getName(), REMOVE, name, "");
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
        if (result <= 0) return -1;
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
            name = toLower(name);
            User* curr = storage.findUser(name);
            if (curr) {
                cout << "Введите пароль: ";
                string pass;
                getline(cin, pass);
                if (pass == curr->getPass()) {
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
                        name = "";
                        continue;
                    }
                }
                else {
                    cout << "Неправльный пароль пользователя\n" << endl;
                    name = "";
                    continue;
                }
            }
            else {
                cout << "Такой пользователь не найден\n" << endl;
                name = "";
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