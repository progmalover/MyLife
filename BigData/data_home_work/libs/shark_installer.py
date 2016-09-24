#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os

class SharkInstaller:
    #@singleton()
    def __init__(self):
        pass

    def shell_command(command):
        child = subprocess.Popen(command, stdout=subprocess.PIPE,
                stderr=subprocess.PIPE, env={"PATH":prefix})

        ret, err = child.communicate()
        if err:
            response['ret'] = -1
            response['msg'] = err.partition(":")[2]
        else:
            response[]

    def run(self):
        print "run !"

