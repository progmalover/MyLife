#!/bin/bash

'''
overwrite
'''

class Parent(object):
    def func(self):
        print 'Parent\n'


class Child(Parent):
    def func(self):
        print 'Child\n'


def main():
    c = Child()
    c.func()


if __name__ == '__main__':
    main()
