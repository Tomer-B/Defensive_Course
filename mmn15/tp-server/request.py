import select
import struct
from threading import Thread, Event
from response import *

import logging
logger = logging.getLogger(__name__)

class RequestHandler(object):
    def __init__(self, socket, server):
        self._socket = socket
        self._server = server
        self._finished = False
        self._stop_event = Event()
        self._thread = Thread(target=self.__start)
        self._thread.start()

    def _get_data_from_sock(self, data_len):
        received_data = b''
        while (len(received_data) != data_len) and ((not self._stop_event.is_set()) or (not self._finished)):
            rlist, _, _ = select.select([self._socket], [], [], 0)
            if rlist:
                new_data = self._socket.recv(data_len-len(received_data))
                assert len(new_data) != 0, 'Client socket closed'
                received_data += new_data
        return received_data

    def _handle_client(self):
        try:
            request_header = self._get_data_from_sock(ServerRequest.REQUEST_HEADER_LEN)
            request = ServerRequest.get_server_request_from_header(self._server, request_header, self._server.version)
            payload = self._get_data_from_sock(request._payload_len)
            request.set_payload(payload)
            response = request.run_handler_and_get_response()
            return response
        except Exception as e:
            logger.error(f'[-] Failed to handle client request')
            return GeneralErrorResponse(self._server.version)

    def __start(self):
        try:
            response = self._handle_client()
            self._socket.send(response.pack())
            self._finished = True
        finally:
            self._socket.close()
            self._finished = True

    def stop(self):
        self._stop_event.set()


def handle_register_client(request):
    try:
        logger.info(f'Handling register client request')
        name = request._payload[0:255]
        pubkey = request._payload[255:415]
        assert b'\x00' in name
        client_id = request._server.register_client(name, pubkey)
        return RegisterClientResponse(request._server.version, client_id)
    except Exception as e:
        logger.error('[-] Failewd to register client')
        raise e


def handle_list_clients(request):
    logger.info(f'[+] Handling client list request')
    clients = request._server.get_client_list(request._client_id)
    return ClientListResponse(request._server.version, clients)


def handle_get_pubkey(request):
    logger.info(f'[+] Handling public key request')
    key = request._server.get_client_pubkey(request._payload)
    return PubkeyResponse(request._server.version, request._payload, key)


def handle_send_message(request):
    logger.info(f'[+] Handling message request')
    dest_client_id = request._payload[0:16]
    message_type = request._payload[16:17]
    content_size = request._payload[17:21]
    content = request._payload[21:request._payload_len]
    message_id = request._server.send_message(request._client_id, dest_client_id, message_type, content_size, content)
    return SendMessageResponse(request._server.version, dest_client_id, message_id)


def handle_get_messages(request):
    logger.info(f'[+] Handling get messsages request')
    messages = request._server.get_unread_messages(request._client_id)
    return GetMessagesResponse(request._server.version, messages)


class ServerRequest(object):
    _REQUEST_TYPES = {1000:{'handler':handle_register_client, 'min_payload_len':415},
                     1001:{'handler':handle_list_clients, 'min_payload_len':0},
                     1002:{'handler':handle_get_pubkey, 'min_payload_len':16},
                     1003:{'handler':handle_send_message, 'min_payload_len':21},
                     1004:{'handler':handle_get_messages, 'min_payload_len':0}}
    REQUEST_HEADER_LEN = 23
    CLIENT_ID_LEN = 16
    REQUEST_STRUCT = '<chI'

    def __init__(self, server, client_id, version, request_code, payload_len):
        self._server = server
        self._client_id = client_id
        self._version = version
        self._request_code = request_code
        self._payload_len = payload_len
        self._payload = None
        self._server.update_last_seen(self._client_id)

    @classmethod
    def validate_request(cls, client_id, version, request_code, payload_len, server_version=None):
        try:
            assert len(client_id) == 16
            assert int(request_code) in cls._REQUEST_TYPES
            assert int(payload_len) >= cls._REQUEST_TYPES[request_code]['min_payload_len']
        except Exception as e:
            logger.error('[-] Server request Invalid')
            raise e

    @classmethod
    def validate_payload(cls, payload_len, payload):
        try:
            assert len(payload) == int(payload_len)
        except Exception as e:
            logger.error('[-] Payload size does not match')
            raise e

    @classmethod
    def get_server_request_from_header(cls, server, message, server_version=None):
        try:
            logger.info(f'[+] Handling request!')
            assert len(message) == cls.REQUEST_HEADER_LEN
            client_id = message[0:cls.CLIENT_ID_LEN]
            logger.info(f'\tClient ID: {client_id}')
            version, request_code, payload_len = struct.unpack(cls.REQUEST_STRUCT, message[cls.CLIENT_ID_LEN:cls.REQUEST_HEADER_LEN])
            cls.validate_request(client_id, version, request_code, payload_len, server_version)
        except Exception as e:
            logger.error('[-] Failed to validate')
            raise e
        return cls(server, client_id, version, request_code, payload_len)

    def set_payload(self, payload):
        self.validate_payload(self._payload_len, payload)
        self._payload = payload

    def run_handler_and_get_response(self):
        try:
            return self._REQUEST_TYPES[self._request_code]['handler'](self)
        except Exception as e:
            logger.error(f'[-] Failed to handle request:\n\t{e}')
            return GeneralErrorResponse(self._server.version)
