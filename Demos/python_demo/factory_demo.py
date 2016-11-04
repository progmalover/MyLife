#!/usr/bin/python

import os
import string

class Factory:
    def createProduct():
        pass


class RedFactory(Factory):
    def __init__(self):
        self.product = 'red'

    def createProduct(self):
        return self.product


class BlueFactory(Factory):
    def __init__(self):
        self.product = 'blue'

    def createProduct(self):
        return self.product


def doProduct(f):
    return f.createProduct()


def main():
    fr = RedFactory()
    fb = BlueFactory()

    print doProduct(fr)
    print doProduct(fb)


if __name__=='__main__':
    main()
