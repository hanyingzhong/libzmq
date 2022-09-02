#!/usr/bin/python
# -*-coding:utf-8-*-

import zmq
import time

context = zmq.Context()
socket = context.socket(zmq.SUB)
socket.setsockopt_string(zmq.SUBSCRIBE, '1/')
socket.connect("tcp://127.0.0.1:5556")
reply = context.socket(zmq.PUB)
reply.connect("tcp://127.0.0.1:5557")

idx = 0

while True:
    msg = socket.recv_multipart()
    print(msg)
    msg = str(idx)
    reply.send_multipart((b'REQ10000', b'result=9999999999999999999999' + msg.encode()))
    idx = idx + 1
