#!/usr/bin/env python

# __call__ make class tobe callable:
class Animal:
    def __init__(self, name):
        self.name = name

    def __call__(self, food):
        self.food = food

# __iter__ make class tobe iteratorable:
class Fibs:
    def __init__(self):
        self.a = 0
        self.b = 1

    def next(self):
        self.a, self.b = self.b, self.a+self.b
        return self.a

    # __iter__ make object into iteratorable, which is object looks like list.
    # If no __iter__ function defined, this is illlegal: `for iter in fibs:`
    # format of method is like this:
    def __iter__(self):
        return self

def main():
    # __call__
    cat = Animal('cat')
    cat('fish')

    # __iter__
    fibs = Fibs()
    for f in fibs:


    print '%s eat %s' % (cat.name, cat.food)


if __name__ == '__main__':
    main()
