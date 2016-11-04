#!/usr/bin/python

import os

s = None

class Singleton:
    def __init__(self):
        self.count = 0

    def one(self):
        self.count = 1
        print self.count

    def two(self):
        self.count = 2
        print self.count

    def three(self):
        self.count = 3
        print self.count

def getSingleton():
    global s
    if not s:
        s = Singleton()
    return s


def decorateSingleton(cls, *arg, **kw):
    instance={}
    def getInstance():
        if cls not in instance:
            instance[cls] = cls(*arg, **kw)
        return instance[cls]
    return getInstance


@decorateSingleton
class Test(object):
    pass


def main():
    getSingleton().one()
    getSingleton().two()
    getSingleton().three()

    t = Test()

if __name__ == '__main__':
    main()
