#!/usr/bin/env python
# -*- coding: utf-8 -*-

from amqplib import client_0_8 as amqp
import process

"""
AMQP is rabiit MQ python library.

Same as xorplus RPC, as server
"""
# 1. register RPC URL using amqp.Connection()
conn = amqp.Connection(
    host="localhost:5672",
    userid="guest",
    password="guest",
    virtual_host="/",
    insist=False)
# 2. use conn.channel() get channel we use.
chan = conn.channel()

# 3. declare, this is same as xorplus rtmgr register process,
# as APIs need to register to rtmgr to let other modules to get it.
#
# declare queue
# declare exchange
#
# Rabbit MQ:
# -- channel --
#      |      ...
# -- queue --
#   |     |
# room1 room2 ...
#   |     |
#  key   key
chan.queue_declare(
    queue="po_box",
    durable=True,
    exclusive=False,
    auto_delete=False)
chan.exchange_declare(
    exchange="sorting_room",
    type="direct",
    durable=True,
    auto_delete=False,)

# 4. bind queue and exchange method into channel.
chan.queue_bind(
    queue="po_box",
    exchange="sorting_room",
    routing_key="testkey")

def recv_callback(msg):
    #TODO
    #print msg.body
    process.starup(msg.body)

# 5. define consumer
# queue is used to dispatch different request.
# no_ack is used for present ack or not.
# callback is call back function.
# consumer_tag is used for log or sth.
chan.basic_consume(
    queue='po_box',
    no_ack=True,
    callback=recv_callback,
    consumer_tag="testtag")

# 6. use chann.wait()
while True:
    chan.wait()

#chan.basic_cancel("testtag")
#chan.close()
#conn.close()
