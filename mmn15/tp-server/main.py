from messageu_server import MessageUServer
import logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

def main():
    server = MessageUServer()
    server.start()

if __name__ == '__main__':
    main()
