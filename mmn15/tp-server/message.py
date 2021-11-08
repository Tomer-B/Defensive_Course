import os
import logging

logger = logging.getLogger(__name__)
BYTE_ORDER = 'little'

class Message(object):
    def __init__(self, message_id, dst_client, src_client, message_type, content):
        self.message_id = message_id
        self.dst_client = dst_client
        self.src_client = src_client
        self.message_type = message_type
        self.content = content

    @classmethod
    def create_message(cls, src_client_id, dest_client_id, message_type, content_size, content):
        message_id = os.urandom(4)
        return cls(message_id, dest_client_id, src_client_id, message_type, content)

    def pack(self):
        return self.src_client+self.message_id+self.message_type+len(self.content).to_bytes(4, BYTE_ORDER)+self.content
