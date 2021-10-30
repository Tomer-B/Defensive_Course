import uuid
import select
import struct
import logging
from threading import Thread, Event
from objects import *

logger = logging.getLogger(__name__)

class UserHandler(object):
    def __init__(self, socket, server):
        self._socket = socket
        self._server = server
        self._done = False
        self._stop_event = Event()
        self._thread = Thread(target=self.__start)
        self._thread.start()

    def _get_data_from_sock(self, socket, data_len):
        logger.info(f'Waiting for {data_len} bytes of data')
        received_data = b''
        while (len(received_data) != data_len) and ((not self._stop_event.is_set()) or (not self._done)):
            rlist, _, _ = select.select([self._socket], [], [], 0)
            if rlist:
                new_data = socket.recv(data_len-len(received_data))
                logger.info(f'Got new {len(new_data)} bytes of data')
                assert len(new_data) != 0, 'Client socket closed'
                received_data += new_data
        logger.info(f'Got {data_len} bytes of data')
        return received_data

    def _handle_user(self):
        try:
            request_header = self._get_data_from_sock(self._socket, ServerRequest.REQUEST_HEADER_LEN)
            request = ServerRequest.get_server_request_from_header(self._server, request_header, self._server.version)
            payload = self._get_data_from_sock(self._socket, request._payload_len)
            request.set_payload(payload)
            response = request.run_handler_and_get_response()
            return response
        except Exception as e:
            logger.error(f'Couldn\'t handle user request')
            return GeneralErrorResponse(self._server.version)

    def __start(self):
        try:
            response = self._handle_user()
            self._socket.send(response.serialize())
            self._done = True
        finally:
            self._socket.close()
            self._done = True

    def stop(self):
        self._stop_event.set()


def handle_register_user(request):
    try:
        logger.info(f'Handling register user request')
        name = request._payload[0:255]
        pubkey = request._payload[255:415]
        assert b'\x00' in name
        user_id = request._server.register_user(name, pubkey)
        return RegisterUserResponse(request._server.version, user_id)
    except Exception as e:
        logger.error('Couldn\'t register user')
        raise e


def handle_list_users(request):
    logger.info(f'Handling list users request')
    users = request._server.get_user_list(request._client_id)
    return UserListResponse(request._server.version, users)


def handle_get_pubkey(request):
    logger.info(f'Handling get pubkey request')
    key = request._server.get_user_pubkey(request.payload)
    return PubkeyResponse(request._server.version, request.payload, key)


def handle_send_message(request):
    logger.info(f'Handling send message request')
    dest_client_id = request.payload[0:16]
    message_type = request.payload[16:17]
    content_size = request.payload[17:21]
    content = request.payload[21:request._payload_len]
    message_id = request._server.send_message(request._client_id, dest_client_id, message_type, content_size, content)
    return SendMessageResponse(request._server.version, dest_client_id, message_id)


def handle_get_messages(request):
    logger.info(f'Handling get messsages request')
    messages = request._server.get_unread_messages(request._client_id)
    return GetMessagesResponse(request._server.version, messages)


class ServerRequest(object):
    _REQUEST_TYPES = {1000:{'handler':handle_register_user, 'min_payload_len':415},
                     1001:{'handler':handle_list_users, 'min_payload_len':0},
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
            if server_version is not None:
                assert version == server_version
        except Exception as e:
            logger.error('Server request is not valid')
            raise e

    @classmethod
    def validate_payload(cls, payload_len, payload):
        try:
            assert len(payload) == int(payload_len)
        except Exception as e:
            logger.error('Payload size does not match declared size')
            raise e

    @classmethod
    def get_server_request_from_header(cls, server, message, server_version=None):
        try:
            logger.info(f'Handling new request')
            assert len(message) == cls.REQUEST_HEADER_LEN
            client_id = message[0:cls.CLIENT_ID_LEN]
            version, request_code, payload_len = struct.unpack(cls.REQUEST_STRUCT, message[cls.CLIENT_ID_LEN:cls.REQUEST_HEADER_LEN])
            cls.validate_request(client_id, version, request_code, payload_len, server_version)
        except Exception as e:
            logger.error('Couldn\'t parse handle')
            raise e
        return cls(server, client_id, version, request_code, payload_len)

    def set_payload(self, payload):
        logger.info(f'Setting request payload')
        self.validate_payload(self._payload_len, payload)
        self._payload = payload

    def run_handler_and_get_response(self):
        try:
            return self._REQUEST_TYPES[self._request_code]['handler'](self)
        except Exception as e:
            logger.error(f'Couldn\'t handle request:\n{e}')
            return GeneralErrorResponse(self._server.version)