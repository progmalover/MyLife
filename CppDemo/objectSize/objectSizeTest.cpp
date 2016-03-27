#include "iostream"

using namespace std;

class NullObject
{
};

class OneObject
{
private:
    int i;
};

class FourObject
{
private:
    int i[10];
};

class FunctionObject
{
public:
    virtual void func(void)
    {
    }
};

int main(int argv, const char* argc[])
{
    NullObject *nb = new NullObject;
    OneObject *ob = new OneObject;
    FourObject *fb = new FourObject;
    FunctionObject *funcb = new FunctionObject;

    cout << "size of nb = " << sizeof(*nb) << endl;
    cout << "size of ob = " << sizeof(*ob) << endl;
    cout << "size of fb = " << sizeof(*fb) << endl;
    cout << "size of funcb = " << sizeof(*funcb) << endl;

    delete nb;
    delete ob;
    delete fb;
    delete funcb;
}
