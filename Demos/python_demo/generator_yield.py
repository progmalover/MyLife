#!/usr/bin/env python

# Generator:
# mygenerator is not list, but could be treat as a function and call function
# once.
mygenerator = (x*x for x in range(3))

for i in mygenerator:
    print i

# This will not error, but will not work, so j will not print.
for j in mygenerator:
    print j


# "yield" is to construct generator:
# so createGenerator == mygenerator
def createGenerator():
    mylist = range(3)
    for i in mylist:
        yield i*i

mygenerator = createGenerator() # mygenerator become a generator object
print(mygenerator)

for i in mygenerator:
    print i

# also not work
for j in mygenerator:
    print j
