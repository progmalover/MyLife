#include "list"
#include "iostream"
#include "algorithm"

using namespace std;

int func(list<int> &l)
{
    if (l.size() == 0) {
        return -1;
    }

    list<int>::iterator head = l.begin();
    list<int>::iterator back = l.end()--;

    while(back != head) {
        int temp = *head;
        l.pop_front();
        l.insert(back, temp);
        head = l.begin();
    }
}

void func2(list<int> &l)
{
    reverse(l.begin(), l.end());
}

int main(int argc, const char* argv[])
{
    int testlist[] = {1,2,3,4,5};
    list<int> test(testlist, testlist + sizeof(testlist)/sizeof(int));

    for (list<int>::iterator iter = test.begin(); iter != test.end(); iter++) {
        cout << *iter << " ";
    }
    cout << endl;

    func2(test);

    for (list<int>::iterator iter = test.begin(); iter != test.end(); iter++) {
        cout << *iter << " ";
    }
    cout << endl;
}
