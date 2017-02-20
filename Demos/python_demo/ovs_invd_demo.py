#!/usr/bin/python
#
# @Author Sam
#
# Ovs-invd is to store some system information into tables bellow of
# Switch_Inventory database.
#   Cpu_Memory_Data
#   Hardware_Flow

import argparse
import re
import subprocess
import sys
import time
import types
import json

import ovs.dirs
import ovs.util
import ovs.daemon
import ovs.unixctl.server
import ovs.vlog


VERSION = "1.0"
root_prefix = ""
__pychecker__ = 'no-reuseattr'  # Remove in pychecker >= 0.8.19.
vlog = ovs.vlog.Vlog("ovs-invd")
exiting = False
maxHwflowInOneJson = 1000


def ovs_invctl(args_list):
    cmd = ["ovs-invctl", "-voff"] + args_list
    output = subprocess.Popen(cmd, stdout=subprocess.PIPE).communicate()
    if len(output) == 0 or output[0] == None:
        output = ""
    else:
        output = output[0].strip()
    return output


def ovs_appctl(args_list):
    cmd = ["ovs-appctl"] + args_list
    output = subprocess.Popen(cmd, stdout=subprocess.PIPE).communicate()
    if len(output) == 0 or output[0] == None:
        output = ""
    else:
        output = output[0].strip()
    return output


def ovs_vsctl(args_list):
    cmd = ["ovs-vsctl", "-vconsole:off"] + args_list
    output = subprocess.Popen(cmd, stdout=subprocess.PIPE).communicate()
    if len(output) == 0 or output[0] == None:
        output = ""
    else:
        output = output[0].strip()
    return output


def unixctl_exit(conn, unused_argv, unused_aux):
    global exiting
    exiting = True
    conn.reply(None)

    
def uptime():
    cmd = ["uptime"]
    output = subprocess.Popen(cmd, stdout=subprocess.PIPE).communicate()
    if len(output) == 0 or output[0] == None:
        output = ""
    else:
        output = output[0].strip()
    return output

    
def top(arg):
    cmd = ["top", "-b", "-n", "1"]
    output = subprocess.Popen(cmd, stdout=subprocess.PIPE).communicate()
    if len(output) == 0 or output[0] == None:
        output = ""
    else:
        output = output[0].splitlines()
        output = output[arg].strip()
    return output

    
def setCpuMemData():
    ### Add inventory database feature here
    # uptime > Switch_Inventory.Cpu_Memory_Data
    uptime_res = uptime()
    ovs_invctl(["set-data-uptime", uptime_res])

    # top cup
    cpu_res = top(2)
    ovs_invctl(["set-data-cpu", cpu_res])

    # top tasks
    tasks_res = top(1)
    ovs_invctl(["set-data-tasks", tasks_res])

    # top memory
    mem_res = top(3)
    ovs_invctl(["set-data-memory", mem_res])


def setHwFlowData():
    hwflows = ovs_appctl(['pica/dump-flows', '--sort'])
    hwflows = hwflows.splitlines()
    reslist = []
    resCount = 0

    for i in range(0, len(hwflows)-1) :
        if hwflows[i] != None :
            if re.match('#', hwflows[i]) :
                reslist.append(hwflows[i])
                resCount += 1
            if resCount >= maxHwflowInOneJson :
                # store maxHwflowInOneJson of reslist into one json object
                encodedjson = json.dumps(reslist)
                ovs_invctl(['add-hwflow', encodedjson])
                reslist = []
                resCount = 0
        else :
            break

    if len(reslist) > 0 :
        # store rest of reslist into one json object
        encodedjson = json.dumps(reslist)
        ovs_invctl(['add-hwflow', encodedjson])


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--root-prefix", metavar="DIR",
                        help="Use DIR as alternate root directory"
                        " (for testing).")
    parser.add_argument("--version", action="version",
                        version="%s %s" % (ovs.util.PROGRAM_NAME, VERSION))

    ovs.vlog.add_args(parser)
    ovs.daemon.add_args(parser)
    args = parser.parse_args()
    ovs.vlog.handle_args(args)
    ovs.daemon.handle_args(args)

    global root_prefix
    if args.root_prefix:
        root_prefix = args.root_prefix

    ovs.daemon.daemonize()

    ovs.unixctl.command_register("exit", "", 0, 0, unixctl_exit, None)
    error, unixctl = ovs.unixctl.server.UnixctlServer.create(None,
                                                             version=VERSION)
    if error:
        ovs.util.ovs_fatal(error, "could not create unixctl server", vlog)

    while True:
        unixctl.run()
        if exiting:
            break

        # Process Cpu Memory Data
        setCpuMemData()

        # Get hwflow_enable: false/true
        hwflow_enable = False
        showList = ovs_invctl(['show']).splitlines()
        for item in showList:
            if re.search('hwflow_enable: true', item):
                hwflow_enable = True

        # Process Hardware Flow Data
        if hwflow_enable:
            ovs_invctl(['clear-hwflows'])
            setHwFlowData()

        # Poll
        interval = int(ovs_invctl(['get-hwflow-interval']))
        if interval == None or interval < 10 or interval > 600:
            interval = 600*1000
        else :
            interval = interval*1000

        poller = ovs.poller.Poller()
        unixctl.wait(poller)
        poller.timer_wait(interval)
        poller.block()

    unixctl.close()


if __name__ == '__main__':
    try:
        main()
    except SystemExit:
        # Let system.exit() calls complete normally
        raise
    except:
        vlog.exception("traceback")
        sys.exit(ovs.daemon.RESTART_EXIT_CODE)
