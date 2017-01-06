#include "iostream"

using namespace std;

class Base {
public:
    virtual void func(void) {
        cout << "Base" << endl;
    }
};

class Child : public Base {
public:
    virtual void func(void) {
        cout << "Child" << endl;
    }
};

class Base2 {
public:
    void func(void) {
        cout << "Base2" << endl;
    }
};

class Child2 : public Base2 {
public:
    void func(void) {
        cout << "Child2" << endl;
    }
};

int main() {
    Base *b = new Child();
    b->func();  // Child

    Child *c = (Child *)b;
    c->func();  // Child

    //ERROR
    //Base *b2 = new Base();
    //Child *c2 = (Base *)b2;
    //c2->func();

    Base2 *b3 = new Child2();
    b3->func(); // Base2

    Child2 *c3 = (Child2 *)b3;
    c3->func(); // Child2

    return 0;
}
