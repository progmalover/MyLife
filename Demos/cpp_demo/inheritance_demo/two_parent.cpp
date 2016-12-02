#include "iostream"

using namespace std;

class Base {
public:
    virtual void func()
    {
        cout << "S" << endl;
        return ;
    }
};

class ParentA : public Base {
public:
    virtual void super_function()
    {
        cout << "A" << endl;
        return ;
    }
};

class ParentB : public Base {
public:
    virtual void super_function()
    {
        cout << "B" << endl;
        return ;
    }
};

class Child : public ParentA, public ParentB {
public:
    void child_function()
    {
        cout << "C" << endl;
        return ;
    }

    void super_function()
    {
        cout << "D" << endl;
        return ;
    }
};

int
main(const int argv, const char** argc)
{
    Child *c = new Child;
    c->child_function();
    c->super_function();
    // c->func(); // got error

    return 0;
}
