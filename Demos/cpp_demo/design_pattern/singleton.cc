// Use internal member value to singleton
class Singleton
{
public:
    static Singleton* getInstance();
private:
    Singleton();
    static Singleton *_instance;
}

Singleton::Singleton() : _instance(NULL)
{
}

Singleton* getInstance()
{
    if (_instance == NULL) {
        Lock();
        if (_instance == NULL) {
            _instance = new Singleton;
        }
        Unlock();
    }
}

void useSingletonOne()
{
    Singleton *s = Singleton::getInstance();
}

void useSingletonTemplate()
{
    Widget &w = Singleton<Widget>::getInstance();
}
