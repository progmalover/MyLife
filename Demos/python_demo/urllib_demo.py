#!/usr/bin/env python

'''
### Reference?

1. https://docs.python.org/2/library/urllib.html


### Introduction?

"urllib" is python library for send http request and get response by url. In
python3, is splite into "urllib.request", "urllib.parse", "urllib.error".

'''

import urllib
import sys

def urlopen_example():
    '''
    @Function urllib.urlopen(url[, data[, proxies[, context]]]).
    @Param url: request url.
    @Param data: HTTP method, default is GET.
    @Param proxies: map of proxies, like {'http': 'http://www.someproxy.com:3128'}.
    @Param context: SSL context, string format, may be SSL key.
    @Return: file-like object, contains HTTP message. The object supports
        following method: file-like method as read, readline, readlines, fileno,
        close; RTTI method as info; and HTTP related method as getcode, geturl.
    @Exception: IOError
    '''
    some_url = 'http://www.bing.com'
    # Use http://www.someproxy.com:3128 for HTTP proxying
    # proxies = {'http': 'http://www.someproxy.com:3128'}
    # filehandle = urllib.urlopen(some_url, proxies=proxies)
    # Don't use any proxies
    # filehandle = urllib.urlopen(some_url, proxies={})
    # Use proxies from environment - both versions are equivalent
    # filehandle = urllib.urlopen(some_url, proxies=None)
    try:
        filehandle = urllib.urlopen(some_url)
    except Exception as err:
        print err
    finally:
        # raise Exception('%s' % (sys._getframe().f_lineno))
        pass

    print filehandle
    print filehandle.readline()
    print filehandle.getcode()


def urlretrieve_example():
    '''
    @ Function urllib.urlretrieve(url[,filename[,reporthook[,data]]])
    @ Param filename: file name with path, file is to store HTTP response.
    ...
    @ Return: two value tuple (filename, mine_hdrs)
    '''
    # store as temp file
    filename = urllib.urlretrieve('http://www.google.com.hk/')
    print type(filename)
    print filename[0]
    print filename[1]

    # to clean temp file
    urllib.urlcleanup()


def quote_example():
    '''
    @ Function: to replace some character into browser format.
    '''
    print urllib.quote('/~connolly/')
    print urllib.quote_plus(' / ~connolly/ ')
    print urllib.unquote('/%7Econnolly/')


def urlencode_example():
    params=urllib.urlencode({'spam':1,'eggs':2,'bacon':0})
    print params

    f=urllib.urlopen("http://python.org/query?%s" % params)
    print f.readline()


def main():
    urlopen_example()
    urlretrieve_example()
    quote_example()
    urlencode_example()


if __name__ == '__main__':
    main()
