import os
import time
import uuid
import base64

BYTE_ORDER = 'little'

class Message(object):
    def __init__(self, message_id, to_client, from_client, message_type, content):
        self.message_id = message_id
        self.to_client = to_client
        self.from_client = from_client
        self.message_type = message_type
        self.content = content

    @classmethod
    def gen_new_message(cls, src_client_id, dest_client_id, message_type, content_size, content):
        message_id = os.urandom(4)
        return cls(message_id, dest_client_id, src_client_id, message_type, content)

    def serialize(self):
        return self.from_client+self.message_id+self.message_type+len(self.content).to_bytes(4, BYTE_ORDER)+self.content


class User(object):
    def __init__(self, user_id, name, public_key, last_seen):
        self.user_id = user_id
        self.name = name
        self.public_key = public_key
        self.last_seen = last_seen

    @classmethod
    def create_new_user(cls, name, public_key):
        user_id = uuid.uuid1().bytes
        last_seen = time.time()
        return cls(user_id, name, public_key, last_seen)

    def update_last_seen(self):
        self.last_seen = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))

    def serialize(self):
        assert type(self.user_id) is bytes and len(self.user_id) == 16
        assert type(self.name) is bytes
        return self.user_id+self.name


class ServerResponse(object):
    def __init__(self, version):
        self.version = version

    def serialize(self):
        code_bytes = self.RESPONSE_CODE.to_bytes(2, BYTE_ORDER)
        payload = self.serialize_payload()
        payload_size = len(payload).to_bytes(4, BYTE_ORDER)
        return self.version+code_bytes+payload_size+payload

    def serialize_payload(self):
        raise NotImplementedError()


class GeneralErrorResponse(ServerResponse):
    RESPONSE_CODE = 9000

    def __init__(self, version):
        super().__init__(version)

    def serialize_payload(self):
        return b''


class RegisterUserResponse(ServerResponse):
    RESPONSE_CODE = 2000

    def __init__(self, version, user_id):
        super().__init__(version)
        self.user_id = user_id

    def serialize_payload(self):
        assert type(self.user_id) is bytes and len(self.user_id) == 16
        return self.user_id


class UserListResponse(ServerResponse):
    RESPONSE_CODE = 2001

    def __init__(self, version, users):
        super().__init__(version)
        self.users = users

    def serialize_payload(self):
        users_data = b''
        for user in self.users:
            users_data += user.serialize()
        return users_data


class PubkeyResponse(ServerResponse):
    RESPONSE_CODE = 2002

    def __init__(self, version, client_id, pubkey):
        super().__init__(version)
        self.client_id = client_id
        self.pubkey = pubkey

    def serialize_payload(self):
        assert type(self.client_id) is bytes and len(self.client_id) == 16
        assert type(self.pubkey) is bytes and len(self.pubkey) == 160
        return self.client_id+self.pubkey


class SendMessageResponse(ServerResponse):
    RESPONSE_CODE = 2003

    def __init__(self, version, dest_client_id, message_id):
        super().__init__(version)
        self.dest_client_id = dest_client_id
        self.message_id = message_id

    def serialize_payload(self):
        assert type(self.dest_client_id) is bytes and len(self.dest_client_id) == 16
        assert type(self.message_id) is bytes and len(self.message_id) == 4
        return self.dest_client_id+self.message_id


class GetMessagesResponse(ServerResponse):
    RESPONSE_CODE = 2004

    def __init__(self, version, messages):
        super().__init__(version)
        self.messages = messages

    def serialize_payload(self):
        message_data = b''
        for message in self.messages:
            message_data += message.serialize()
        return message_data