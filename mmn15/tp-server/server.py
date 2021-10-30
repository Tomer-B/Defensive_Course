import time
import socket
import select
import logging
from network import UserHandler
from objects import User, Message

PORT_INFO_FILE = 'port.info'
VERSION_INFO_FILE = 'version.info'
MAX_CLIENTS = 10
BYTE_ORDER = 'little'

logger = logging.getLogger(__name__)

class MessageUServer(object):
    def __init__(self):
        self.port = self._get_port_info()
        self.version = self._get_version_info()
        self._users = {}
        self._messages = {}
        self._socket = None
        self._client_handlers = []

    def _get_port_info(self):
        try:
            with open(PORT_INFO_FILE, 'rb') as f:
                port_str = int(f.read())
        except Exception as e:
            logger.error('Couldn\'t open server port info file')
            raise e
        try:
            port = int(port_str)
            assert (port > 1024 and port < 65536)
        except Exception as e:
            logger.error('Port info doesn\'t contain a valid port number')
            raise e
        return port

    def _get_version_info(self):
        try:
            with open(VERSION_INFO_FILE, 'rb') as f:
                version = int(f.read()).to_bytes(1, BYTE_ORDER)
        except Exception as e:
            logger.error('Couldn\'t open server port info file')
            raise e
        return version

    def _open_server_socket(self):
        try:
            logger.info('Starting server socket')
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.bind(('0.0.0.0', self.port))
            s.listen(MAX_CLIENTS)
        except Exception as e:
            logger.error('Couldn\'t open and bind server socket')
            raise e
        logger.info('Successfully opened server socket')
        return s

    def _check_for_new_clients(self):
        """
        Checks for a new client and create an appropriate channel for it.
        :return: None.
        """
        rlist, _, _ = select.select([self._socket], [], [], 0)
        if rlist:
            client_socket, client_address = self._socket.accept()
            logger.info(f'Accepted new client: {client_address}')
            new_handler = UserHandler(client_socket, self)
            self._client_handlers.append(new_handler)

    def _remove_old_clients(self):
        for handler in self._client_handlers:
            if handler._done:
                self._client_handlers.pop()

    def run_server(self):
        logger.info(f'Started server [Version: {self.version}, Port: {self.port}]')
        self._socket = self._open_server_socket()
        try:
            while(True):
                self._check_for_new_clients()
                self._remove_old_clients()
                time.sleep(0.1)
        finally:
            for handler in self._client_handlers:
                handler.stop()
            self._socket.close()

    def _get_user_from_name(self, name):
        for user in self._users.values():
            if user.name == name:
                return user
        return None

    def register_user(self, name, pubkey):
        if self._get_user_from_name(name) is not None:
            logger.error(f'User exists')
            raise Exception('User already registerd')
        new_user = User.create_new_user(name, pubkey)
        self._users[new_user.user_id] = new_user
        logger.info(f'Registerd new user {new_user.user_id}')
        return new_user.user_id

    def get_user_list(self, ignore_client_id):
        users_to_send = dict(self._users)
        users_to_send.pop(ignore_client_id, None)
        return users_to_send.values()

    def get_user_pubkey(self, client_id):
        return self._users[client_id].public_key

    def send_message(self, src_client_id, dest_client_id, message_type, content_size, content):
        new_message = Message.gen_new_message(src_client_id, dest_client_id, message_type, content_size, content)
        if dest_client_id not in self._messages:
            self._messages[dest_client_id] = []
        self._messages[dest_client_id].append(new_message)
        logger.info(f'Message sent to {dest_client_id} from {src_client_id}')
        raise NotImplementedError()

    def get_unread_messages(self, client_id):
        unread_messages = []
        if client_id in self._messages:
            unread_messages = self._messages[client_id]
        self._messages[client_id] = []
        logger.info(f'Unread messages to {client_id} removed')
        return unread_messages

    def update_last_seen(self, client_id):
        if client_id in self._users:
            self._users[client_id].update_last_seen()