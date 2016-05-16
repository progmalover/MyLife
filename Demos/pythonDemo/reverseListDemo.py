#!/usr/bin/python

arr = [1,2,3,4,5,6,7]

def main():
    print("arr = %s" % arr)
    for i in range(len(arr)-1, -1, -1):
        print("arr[%d] = %d" % (i,arr[i]))

if __name__=='__main__':
    main()
