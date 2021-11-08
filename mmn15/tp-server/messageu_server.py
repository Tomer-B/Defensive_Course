import time
import socket
import select

from serverdb import ServerDB
from request import RequestHandler
from client import Client
from message import Message

PORT_INFO_FILE = 'port.info'
VERSION_INFO_FILE = 'version.info'
MAX_AVAILABLE_CLIENTS = 50
BYTE_ORDER = 'little'
DB_NAME = 'server.db'
MIN_PORT = 1024
MAX_PORT = 65536

import logging
logger = logging.getLogger(__name__)

class MessageUServer(object):
    def __init__(self):
        self._sock = None
        self._request_handlers = []
        self.port = self._initialize_port()
        self.version = self._initialize_version()
        self._server_db = ServerDB(DB_NAME)
        self._clients = self._server_db.load_clients()
        self._messages = self._server_db.load_messages()

    def _initialize_port(self):
        try:
            with open(PORT_INFO_FILE, 'rb') as f:
                port_str = int(f.read())
        except Exception as e:
            logger.error('[-] Failed to initialize port')
            raise e
        try:
            port = int(port_str)
            assert (port > MIN_PORT and port < MAX_PORT)
        except Exception as e:
            logger.error('[-] Port info Invalid')
            raise e
        return port

    def _initialize_version(self):
        try:
            with open(VERSION_INFO_FILE, 'rb') as f:
                version = int(f.read()).to_bytes(1, BYTE_ORDER)
        except Exception as e:
            logger.error('[-] Failed to initialize version')
            raise e
        return version

    def _run_server_tcp_socket(self):
        try:
            logger.info('Starting server socket')
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.bind(('0.0.0.0', self.port))
            s.listen(MAX_AVAILABLE_CLIENTS)
        except Exception as e:
            logger.error('[-] Failed to run server tcp socket')
            raise e
        logger.info('[+] Server socket running')
        return s

    def _accept_new_clients(self):
        """
        Creates a handler for every incoming Client.
        """
        r, _, _ = select.select([self._sock], [], [], 0)
        if r:
            client_sock, client_address = self._sock.accept()
            logger.info(f'[+] Accepted connection from address: {client_address}')
            handler = RequestHandler(client_sock, self)
            self._request_handlers.append(handler)

    def _clean_finished_handlers(self):
        for handler in self._request_handlers:
            if handler._finished:
                self._request_handlers.remove(handler)

    def start(self):
        logger.info(f'[+] Server V.{self.version} running on Port {self.port}')
        self._sock = self._run_server_tcp_socket()
        try:
            while(True):
                self._accept_new_clients()
                self._clean_finished_handlers()
                time.sleep(0.1)
        finally:
            for handler in self._request_handlers:
                handler.stop()
            self._sock.close()

    def _get_client_by_name(self, name):
        for client in self._clients.values():
            if client.name == name:
                return client
        logger.debug(f'[-] Failed to find client by name {name}')
        return None

    def _update_client(self, client):
        if client.client_id not in self._clients:
            self._clients[client.client_id] = client
        self._server_db.update_client(client)

    def _update_messages(self, message):
        if message.dst_client not in self._messages:
            self._messages[message.dst_client] = []
        self._messages[message.dst_client].append(message)
        self._server_db.add_message(message)

    def _remove_client_messages(self, client_id):
        self._messages[client_id] = []
        self._server_db.remove_client_messages(client_id)

    def register_client(self, name, pubkey):
        if self._get_client_by_name(name) is not None:
            logger.error(f'[-] Client already exists')
            raise Exception('Client already exists')
        new_client = Client.create_new_client(name, pubkey)
        self._update_client(new_client)
        self._clients[new_client.client_id] = new_client
        logger.info(f'[+] Registerd New client: {new_client.client_id}')
        return new_client.client_id

    def get_client_list(self, ignore_client_id):
        clients = dict(self._clients)
        clients.pop(ignore_client_id, None)
        return clients.values()

    def get_client_pubkey(self, client_id):
        if client_id not in self._clients:
            raise Exception(f'Client {client_id} not found')
        return self._clients[client_id].public_key

    def send_message(self, src_client_id, dst_client_id, message_type, content_size, content):
        new_message = Message.create_message(src_client_id, dst_client_id, message_type, content_size, content)
        self._update_messages(new_message)
        logger.info(f'[+] Message sent from {src_client_id} to {dst_client_id}')
        return new_message.message_id

    def get_unread_messages(self, client_id):
        messaegs = []
        if client_id in self._messages:
            messaegs = self._messages[client_id]
        self._remove_client_messages(client_id)
        logger.info(f'[+] Revmoed unread messages to {client_id}')
        return messaegs

    def update_last_seen(self, client_id):
        if client_id in self._clients:
            self._clients[client_id].update_last_seen()
            self._update_client(self._clients[client_id])
