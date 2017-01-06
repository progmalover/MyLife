'''
1. http://stackoverflow.com/questions/1294382/what-is-a-global-interpreter-lock-gil
2. http://cenalulu.github.io/python/gil-in-python/
3. https://wiki.python.org/moin/GlobalInterpreterLock

<3> is important, basic speaking, GIL is to make CPython interpreter to run atomically in muti-thread enviroment. F.E:

a = A()
b = B()
a.__del__() # GIL make this statement atomically, in case a.__del__ and b.__del__ run together.
b.__del__()

'''
