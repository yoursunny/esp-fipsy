"""
Sends a .jed file to your ESP32
Example Usage:
python sendJed.py 192.168.1.2 --file 'C:\path\to\yourfiqle.jed'

or

python sendJed.py 192.168.1.2 --port 34779 --file 'C:\path\to\yourfile.jed'

"""

import socket
import argparse
import os

def netcat(host, port, content):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, int(port)))
    s.sendall(content)
    s.shutdown(socket.SHUT_WR)
    while True:
        data = s.recv(4096)
        if not data:
            break
        print(repr(data.decode('utf-8')))
    s.close()

def client(ip, port, filename):
    # Check if file exists
    if not os.path.isfile(filename):
        raise FileNotFoundError(f"File {filename} does not exist.")
    
    # Check if file ends with .jed
    if not filename.endswith('.jed'):
        raise ValueError("File must end with .jed extension.")
    
    # Check if file has non-zero size
    if os.path.getsize(filename) == 0:
        raise ValueError("File is empty.")

    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print("Before connect")
    client_socket.connect((ip, port))
    print("After connect")

    with open(filename, 'rb') as file:
        content = file.read()
        netcat(ip, port, content)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Send a .jed file over TCP and print returned text.')
    parser.add_argument('ip', type=str, help='The IP address of the ESP32 server.')
    parser.add_argument('--port', type=int, default=34779, help='The port number to connect to (default is 34779).')
    parser.add_argument('--file', type=str, required=True, help='The path to the .jed file to send.')

    args = parser.parse_args()
    
    client(args.ip, args.port, args.file)