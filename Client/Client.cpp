#include <iostream>
#include <string>
#include <vector>

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
};

class UserStorage {
    vector<User> users;

public:
    void saveUser(string fileName);
    void loadUsers(string fileName);

    void addUser(User user);
    User* findUser(string name);
    void removeUser(User* user);
};

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
    
}