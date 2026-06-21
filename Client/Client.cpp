#include <iostream>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

enum Role {
    ADMIN,
    USER
};

enum Status {
    ACTIVE,
    INACTIVE
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

    void printAllUsers();
};

void UserStorage::saveUsers(string filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Can't open file\n";
        return;
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
        cerr << "Can't open file\n";
        return;
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
            cout << "Такой пользователь уже есть\n";
            return;
        }
    }
    users.push_back(user);
    cout << "Добавлен пользователь " << user.getName() << endl;
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
        cout << "Такого пользователя не существует\n";
        return;
    }
        
    users.erase(remove(users.begin(), users.end(), *user), users.end());
    cout << "Пользователь " << user->getName() << " удален\n";
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

int main()
{
    setlocale(LC_ALL, "Rus");
    UserStorage storage;
    User admin("lexa;admin;active;");
    User user("ivan;user;inactive;");
    User user1("sergey;user;active;");

    storage.loadUsers("user.txt");
    storage.printAllUsers();

    //storage.saveUsers("user.txt");

    //storage.removeUser(storage.findUser("lexa"));
}