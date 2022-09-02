import time
import zmq


class NcaRpcClient(object):
    def __init__(self, pub_url, sub_url):
        """
        :type pub_url: string
        :type sub_url: string
        """
        self.context = zmq.Context()
        self.pubUrl = pub_url
        self.subUrl = sub_url
        self.req_h = 0
        self.rep_h = 0
        self.connect_pub_server()
        self.topics = set()
        self.connect_sub_server()

    def connect_pub_server(self):
        self.req_h = self.context.socket(zmq.PUB)
        self.req_h.connect(self.pubUrl)

    def connect_sub_server(self):
        self.rep_h = self.context.socket(zmq.SUB)
        self.rep_h.connect(self.subUrl)

    def subscribe(self, topic):
        self.rep_h.setsockopt_string(zmq.SUBSCRIBE, topic)
        self.topics.add(topic)

    def unsubscribe(self, topic):
        self.rep_h.setsockopt_string(zmq.UNSUBSCRIBE, topic)
        self.topics.remove(topic)

    # timeout : m-seconds
    def receive_reply(self, timeout=0):
        try:
            if timeout != 0:
                self.rep_h.setsockopt(zmq.RCVTIMEO, timeout * 1000)
            response = self.rep_h.recv_multipart()
        except zmq.Again:
            print("receive timeout")
            response = "ERROR"
        return response

    def call(self, msg, timeout=0):
        self.req_h.send_multipart([b'1/0', msg.encode()])
        response = self.receive_reply(timeout)
        print(response)
        return response

    def subsocket_close(self):
        for item in self.topics:
            self.rep_h.setsockopt_string(zmq.UNSUBSCRIBE, item)
        self.rep_h.close()

    def pubsocket_close(self):
        self.req_h.close()

    def close(self):
        self.subsocket_close()
        self.pubsocket_close()
        self.context.destroy()

    def async_request(self, topic, msg, sync_flag=False):
        if type(topic) is str:
            self.req_h.send_multipart([topic.encode(), msg.encode()])
        if type(topic) is bytes:
            self.req_h.send_multipart([topic, msg.encode()])

    def sync_request(self, topic, msg, timeout=0):
        self.async_request(topic, msg, sync_flag=True)
        response = self.receive_reply(timeout)
        print(response)
        return response


if __name__ == '__main__':
    pub_url = "tcp://127.0.0.1:5555"
    sub_url = "tcp://127.0.0.1:5558"

    client = NcaRpcClient(pub_url, sub_url)
    client.subscribe("REQ10000")
    time.sleep(1)
    client.call("add..........", timeout=2)

    client.sync_request("1/10", "1234567890", timeout=2)
    client.async_request("1/10", "1234567890")

    client.close()
