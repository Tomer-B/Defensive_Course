import logging
logger = logging.getLogger(__name__)
BYTE_ORDER = 'little'

class ServerResponse(object):
    def __init__(self, version):
        self.version = version

    def pack(self):
        code_bytes = self.CODE.to_bytes(2, BYTE_ORDER)
        payload = self.pack_payload()
        payload_size = len(payload).to_bytes(4, BYTE_ORDER)
        return self.version+code_bytes+payload_size+payload

    def pack_payload(self):
        raise NotImplementedError()


class GeneralErrorResponse(ServerResponse):
    CODE = 9000

    def __init__(self, version):
        super().__init__(version)

    def pack_payload(self):
        return b''


class RegisterClientResponse(ServerResponse):
    CODE = 2000

    def __init__(self, version, client_id):
        super().__init__(version)
        self.client_id = client_id

    def pack_payload(self):
        assert type(self.client_id) is bytes and len(self.client_id) == 16
        return self.client_id


class ClientListResponse(ServerResponse):
    CODE = 2001

    def __init__(self, version, clients):
        super().__init__(version)
        self.clients = clients

    def pack_payload(self):
        clients_data = b''
        for client in self.clients:
            clients_data += client.pack()
        return clients_data


class PubkeyResponse(ServerResponse):
    CODE = 2002

    def __init__(self, version, client_id, pubkey):
        super().__init__(version)
        self.client_id = client_id
        self.pubkey = pubkey

    def pack_payload(self):
        assert type(self.client_id) is bytes and len(self.client_id) == 16
        assert type(self.pubkey) is bytes and len(self.pubkey) == 160
        return self.client_id+self.pubkey


class SendMessageResponse(ServerResponse):
    CODE = 2003

    def __init__(self, version, dest_client_id, message_id):
        super().__init__(version)
        self.dest_client_id = dest_client_id
        self.message_id = message_id

    def pack_payload(self):
        assert type(self.dest_client_id) is bytes and len(self.dest_client_id) == 16
        assert type(self.message_id) is bytes and len(self.message_id) == 4
        return self.dest_client_id+self.message_id


class GetMessagesResponse(ServerResponse):
    CODE = 2004

    def __init__(self, version, messages):
        super().__init__(version)
        self.messages = messages

    def pack_payload(self):
        message_data = b''
        for message in self.messages:
            message_data += message.pack()
        return message_data
