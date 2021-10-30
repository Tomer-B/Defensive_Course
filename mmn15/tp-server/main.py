from server import MessageUServer
import logging

logging.basicConfig(level=logging.INFO)

logger = logging.getLogger(__name__)

def main():
    mus = MessageUServer()
    mus.run_server()

if __name__ == '__main__':
    main()