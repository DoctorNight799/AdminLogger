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

    User(string _name, Role _role, Status _status) : name(_name), role(_role), status(_status) {}

    string toString();

    void loadFromString(string st);
};

string User::toString() {
    return "";
}

void User::loadFromString(string st) {
    int first = st.find(';');
    int second = st.find(';', first + 1);
    int third = st.find(';', second + 1);

    string _name = st.substr(0, first);
    string _role = st.substr(first + 1, second - first - 1);
    string _status = st.substr(second + 1, third - second - 1);

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