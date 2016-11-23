#!/bin/python
#
# <<1>>:<<Dive into Python>>
#
# Here intruduce python built in data types.
# Notice, python only have directory, list, map as container. list could be
# use as vector, stack, heap and so on.

# number(int/float/double...), bool, string is basic data type.
# for number, only notice upcasting is OK.

# string
s = '\\' # will get \
s = r'\\' # will get \\
print '''1.
2. '''
# get
# 1.2.
print '''1.\
2.'''
# get
# 1.
# 2.
'1' + '2' # '12'
'1'*3 # '111'
print '1' '2' # 12
s = 'abc'
print a[0] # a

# directory
# feature: no sequence, case sensitive.
d = {'1':'first', '2':'second'} # this is directory
print d['1']    # access first element
d['1'] = 'modify first' # modify

# list
li = [1,2,3,4]
print li[4],li[-1],li[0:3]  # 0:3 is slice
li[4] = '4'
li.extend([5, 6])
print li[0:-1] # 1,2,3,4,5,6
li.append([7,8])
print li[0:-1] # 1,2,3,4,5,6,[7,8]
print li.index(1) # 0
try {
    li.index(1000) # will be an exception, not None!!!
} catch(e) {
    print e
}
if 1000 in li: # use this to test
    process(li)
else:
    print '1000 is not in li'
# python has no bool(true or false), refer to <<1>>P34
try {
    li.remove(1)
    li.remove(1000)
} catch(e) {
    print e
}
li.pop() # this could get li[-1] and then delete it

li2 = [10,11,12]
li3 = li + li2 # [1,2,3,4,5,6,[7,8],10,11,12]
li2 += [13] # [10,11,12,13]
# list mapping
li = [1,2,3]
[elem*2 for elem in li] # [2,4,6]
join(['%d '%(elem) for elem in li]) # '1 2 3 '

# tuple, is static list
t = (1,2,3,4)
print t[0:-1]
# t.append(5) will get error

