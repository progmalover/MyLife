#!/usr/bin/env python
# -*- coding: utf-8 -*-
#client.py

import sys
import time

import json
from amqplib import client_0_8 as amqp

conn = amqp.Connection(
    host="localhost:5672",
    userid="guest",
    password="guest",
    virtual_host="/",
    insist=False)
chan = conn.channel()

i = 0
while 1:
    #msg = amqp.Message('Message %d' % i)

    #笔记更新
    '''
    data = {
        "noteId" : 1,
        "recvUserId" : 2,
        "title" : "test",
        "updateUserName":"你好",
        "remindCount":10,
        "projectName":"11",
        "token":"b258f5e3809017e371009f32eba7d72fcf51165406ce011951811b53db15b414",
        "isPushed":0,
        "messageType":"NoteUpdated",
    }
    '''

    '''
    data = {
        "projectId": 1,
        "recvUserId": 2,
        "reqUserName": "testUser",
        "reqUserStatus": "pending",
        "remindCount": 10,
        "projectName":"11",
        "token":"b258f5e3809017e371009f32eba7d72fcf51165406ce011951811b53db15b414",
        "messageType":"Apply",
    }
    '''

    data = {
        "projectId": 1,
        "recvUserId": 2,
        "reqUserName": "testUser",
        "reqUserStatus": "",
        "remindCount": 10,
        "projectName": "11",
        "token":"b258f5e3809017e371009f32eba7d72fcf51165406ce011951811b53db15b414",
        "messageType":"Review",
    }

    s = json.dumps(data,ensure_ascii=False)
    print s
    msg = amqp.Message(s)

    msg.properties["delivery_mode"] = 2

    # this is to communicate with server.
    chan.basic_publish(msg,
        exchange="sorting_room",
        routing_key="testkey")
    i += 1
    time.sleep(1)
    break

chan.close()
conn.close()
