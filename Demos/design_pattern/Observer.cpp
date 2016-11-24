/*
 * =====================================================================================
 *
 *       Filename:  Observer.cpp
 *
 *    Description:  Observer demo of <<DaoFaZiRan>>
 *
 *        Version:  1.0
 *        Created:  02/02/2016 01:13:49 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  batmancn,
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>

// Cpp has no library of Observer pattern, we have to code.
// 1. Code Observer, Observer has update function, which is called when Observable object
// change its state.
class Observer
{
public:
    virtual void update(Observable o) = 0;
}

class Observable
{
public:
    // This is used for register observer
    virtual void addObserver(Observer *o)
    {
        _observers.push_back(o);
    }

    virtual void notify()
    {
        // notify all observer in _observers
    }
private:
    List<Observer *> _observers;    // use _ to tell this is private member, same as python
}
