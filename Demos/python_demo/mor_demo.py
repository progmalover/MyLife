#!/bin/python

class Base:
    def base_function(self):
        print "base\n"


class ParentA(Base):
    def func(self):
        print "a\n"


class ParentB(Base):
    def func(self):
        print "b\n"


class Child(ParentA, ParentB):
    def c_func(self):
        print "c\n"


def main():
    c = Child()
    c.c_func()
    c.base_function()
    c.func()


if __name__ == '__main__':
    main()
