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

def main():
    getSingleton().one()
    getSingleton().two()
    getSingleton().three()

if __name__ == '__main__':
    main()
