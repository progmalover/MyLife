"""
Demo for how to use process pool in python
"""
from multiprocessing import Pool
import os, time, random

def long_time_task(name):
    """ C style function name, to run long time task. """
    print 'Run task %s, pid = %s, start time = %s' % name, os.getpid(), time.time()
    time.sleep(random.random() * 3)
    print 'End in %s' % time.time()

if __name__ = '__main__'
    p = Pool()  # default Pool size is 4, you could use Pool(5) to change it
    for i in range(5):
        p.apply_async(long_time_task, args=(i,))
    print 'Start subprocess.'
    p.close()   # use close to disable adding subprocess
    p.join()
    print 'All subprocess done.'
