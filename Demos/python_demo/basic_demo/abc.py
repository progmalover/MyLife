#!/bin/env python

'''
Demo for abc and abstract.
'''

import abc

class Base(object):
    @abc.abstractmethod
    def method(self):
        print "Base\n"


class Child(Base):
    def method(self):
        super(Child, self).method()
        print "Child\n"


def main():
    c = Child()
    c.method()


if __name__ == '__main__':
    main()
