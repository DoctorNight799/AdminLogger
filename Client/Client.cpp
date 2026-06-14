#include <iostream>
#include <string>
#include <vector>

using namespace std;

enum Role {
    Admin,
    User
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

    void loadFromString(vector<string> data);
};

string User::toString() {

}

void User::loadFromString(vector<string> data) {

}


int main()
{
    std::cout << "Client sending info...\n";
}