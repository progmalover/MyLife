#!/usr/bin/python

import os

a = [1,4,7,9]
b = [2,3,4,5]

def main():
    one = []
    two = []

    # 1. merge a and b
    c = a + b
    print('a+b= ', c)

    # 2. sort c
    c.sort()
    print('c= ', c)

    # devide into max and min
    i=0
    while(i < len(c)):
        if (sum(one) > sum(two)):
            one.append(c[i])
            two.append(c[i+1])
        else:
            one.append(c[i+1])
            two.append(c[i])
        i = i+2

    # last, output one and two
    print('one = ', one, ', sum = ', sum(one))
    print('two = ', two, ', sum = ', sum(two))

if __name__=='__main__':
    main()
