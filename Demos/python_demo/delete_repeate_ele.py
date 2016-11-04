#!/usr/bin/python

import os

arr = [1,1,2,3,4,4,4,5,4,2,2,1,4,3,2,4,3]

def main():
    print('arr = %s' % arr)
    arr.sort()

    temp = arr[-1]
    for i in range(len(arr)-2, -1, -1):
        if(temp == arr[i]):
            del arr[i]
        else:
            temp = arr[i]
    print('arr = %s' % arr)

if __name__=='__main__':
    main()
