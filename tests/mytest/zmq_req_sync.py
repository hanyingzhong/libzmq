#!/usr/bin/python
# -*-coding:utf-8-*-

import zmq
import time
import threading

context = zmq.Context()
req = context.socket(zmq.PUB)
req.connect("tcp://127.0.0.1:5555")


def receive_reply():
    context1 = zmq.Context()
    rep = context1.socket(zmq.SUB)
    rep.setsockopt_string(zmq.SUBSCRIBE, 'REQ10000')
    rep.connect("tcp://127.0.0.1:5558")
    while True:
        msg = rep.recv_multipart()
        print(msg)


def get_reply_socket():
    context1 = zmq.Context()
    rep_sock = context1.socket(zmq.SUB)
    rep_sock.setsockopt(zmq.LINGER, 0)
    rep_sock.setsockopt(zmq.RCVTIMEO, 2000)
    rep_sock.setsockopt_string(zmq.SUBSCRIBE, 'REQ10000')
    rep_sock.setsockopt_string(zmq.SUBSCRIBE, 'handshake')
    rep_sock.connect("tcp://127.0.0.1:5558")
    return rep_sock


if __name__ == '__main__':
    # th = threading.Thread(target=receive_reply, args=())
    # th.start()
    rep = get_reply_socket()
    #wait rep socket connect to server
    time.sleep(1)

    while True:
        req.send_multipart((b'1/0', b'aaaaaaaaaaaaaaaaaaaaaaaaa'))
        # rep.setsockopt_string(zmq.SUBSCRIBE, 'REQ10000')
        try:
            msg = rep.recv_multipart()
            print(msg)
        except zmq.Again:
            print("=====wait response expired========")
        time.sleep(1)
