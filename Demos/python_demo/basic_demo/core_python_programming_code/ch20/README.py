#!/bin/bash
#-*-coding:utf-8-*-

# Reference:
# 1. <python核心编程>

import urlparse # http://blog.csdn.net/hzrandd/article/details/10107865
import urllib # http://blog.csdn.net/adrianfeng/article/details/5864510

'''
URL:
F.E. https://console.aws.amazon.com/billing/home?#/bill?year=2016&month=12
    https://us-west-2.console.aws.amazon.com/ec2/v2/home?region=us-west-2#Instances:
Format: prot_sch://net_loc/path;params?query#frag
    prot_sch: protocol schema, F.E HTTP, FTP etc.
    net_loc:server location, F.E. 10.10.50.50. aws.com, gateway:gateway@10.10.50.50:6640 etc.
        Format: user:passwd@host:port.
    path: static file or cgi program path in server, F.E. /index.html, /manage/login etc.
    params: param name, F.E. bill, home etc.
    query: keys used for SELECT(GET, POST), F.E. year=2016, month=12, linked by &.
    frag: used for devide params, this is auto used by urllib.splite().
'''

def _urlparse():
    '''
    urlparse.urlparse(urlstr, defPathSchema=None, allowFrag=None)
    urlparse.urlunparse(str)
    urllib.urljoin(baseurl, newurl, allowFrag=None)
    f = urllib.urlopen(urlstr, postQuarryData)
        API of f, refer to <1>P559
    urllib.urlretrieve(urlstr, localfile=None, downloadStatusHook=None)
        Refer to <1>P559
    urllib.quote() urllib.quote_plus(), refer to <2>
    urllib.urlencode()
    '''
    print urlparse.urlparse('https://us-west-2.console.aws.amazon.com/ec2/v2/home?region=us-west-2#Instances:')
    print urlparse.urljoin('https://us-west-2.console.aws.amazon.com/ec2/v2/home?region=us-west-2#Instances:', 'https://us-west-2.console.aws.amazon.com/ec2/v2/home?region=us-west-2#Instances:')

    # For 'cppjava.cn', urlopen treat this as file.
    # For http://cppjava.cn, urlopen treat this as URL.
    f = urllib.urlopen('http://cppjava.cn')
    print f


def _urlllib2():
    '''

    '''


def main():
    _urlparse()

if __name__ == '__main__':
    main()
