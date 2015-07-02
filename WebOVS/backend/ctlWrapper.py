#!/usr/bin/env python

import subprocess
import os
from config import database, prefix

def execute(commands):
    command = ['ovs-vsctl', '--db=%s' % database]
    sub = subprocess.Popen(command+commands, stderr=subprocess.PIPE,
            stdout=subprocess.PIPE, env={"PATH":prefix})
# FIXME where should error be handled?
    return sub.communicate()

def of_execute(commands):
    command = ['ovs-ofctl']
    sub = subprocess.Popen(command+commands, stderr=subprocess.PIPE,
            stdout=subprocess.PIPE, env={"PATH":prefix})
    return sub.communicate()
