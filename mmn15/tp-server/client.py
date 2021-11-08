import time
import uuid

class Client(object):
    def __init__(self, client_id, name, public_key, last_seen):
        self.client_id = client_id
        self.name = name
        self.public_key = public_key
        self.last_seen = last_seen

    @classmethod
    def create_new_client(cls, name, public_key):
        client_id = uuid.uuid1().bytes
        last_seen = time.time()
        return cls(client_id, name, public_key, last_seen)

    def update_last_seen(self):
        self.last_seen = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))

    def pack(self):
        assert type(self.client_id) is bytes and len(self.client_id) == 16
        assert type(self.name) is bytes
        return self.client_id+self.name

