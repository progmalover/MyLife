#!/usr/bin/env python

import threading
from time import sleep, ctime

loops = [ 4, 2 ]

def loop(nloop, nsec):
    print 'start loop', nloop, 'at:', ctime()
    sleep(nsec)
    print 'loop', nloop, 'done at:', ctime()

def main():
    print 'starting at:', ctime()
    threads = []
    nloops = range(len(loops))

    for i in nloops:
        # target(*args)
        t = threading.Thread(target=loop,
	    args=(i, loops[i]))

        threads.append(t)

    for i in nloops:            # start threads
        threads[i].start()

    # join is important, main thread call child.join, which
    # means main thread wait child quit.
    #
    # So join will be:
    # def join(self):
    #     self.wait(self.pid)
    #
    # Refer to pthread join function:
    # extern int pthread_join __P ((pthread_t __th, void **__thread_return));
    # http://www.cnblogs.com/stli/archive/2010/03/11/1683808.html
    for i in nloops:            # wait for all
        threads[i].join()       # threads to finish

    print 'all DONE at:', ctime()

if __name__ == '__main__':
    main()
